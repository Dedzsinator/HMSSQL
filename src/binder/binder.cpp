#include <iostream>
#include <unordered_set>

#include "../include/binder/binder.h"
#include "../include/binder/bound_statement.h"
#include "../include/binder/statement/create_statement.h"
#include "../include/binder/statement/delete_statement.h"
#include "../include/binder/statement/insert_statement.h"
#include "../include/binder/statement/select_statement.h"
#include "../include/binder/tokens.h"
#include "../include/common/exception.h"
#include "../include/common/logger.h"
#include "../include/common/util/string_util.h"
#include "fmt/format.h"
#include "pg_definitions.hpp"
#include "postgres_parser.hpp"
#include "../include/type/decimal_type.h"

namespace hmssql {

Binder::Binder(const Catalog &catalog) : catalog_(catalog) {}

auto Binder::Parse(const std::string &query) -> duckdb_libpgquery::PGNode* {
  parser_.Parse(query);
  if (!parser_.success) {
    throw Exception(fmt::format("Query failed to parse: {}", parser_.error_message));
  }
  return reinterpret_cast<duckdb_libpgquery::PGNode*>(parser_.parse_tree);
}

void Binder::ParseAndSave(const std::string &query) {
  parser_.Parse(query);
  if (!parser_.success) {
    LOG_INFO("Query failed to parse!");
    throw Exception(fmt::format("Query failed to parse: {}", parser_.error_message));
    return;
  }

  if (parser_.parse_tree == nullptr) {
    LOG_INFO("parser received empty statement");
    return;
  }

  SaveParseTree(parser_.parse_tree);
}

auto Binder::IsKeyword(const std::string &text) -> bool { return duckdb::PostgresParser::IsKeyword(text); }

auto Binder::KeywordList() -> std::vector<ParserKeyword> {
  auto keywords = duckdb::PostgresParser::KeywordList();
  std::vector<ParserKeyword> result;
  for (auto &kw : keywords) {
    ParserKeyword res;
    res.name_ = kw.text;
    switch (kw.category) {
      case duckdb_libpgquery::PGKeywordCategory::PG_KEYWORD_RESERVED:
        res.category_ = KeywordCategory::KEYWORD_RESERVED;
        break;
      case duckdb_libpgquery::PGKeywordCategory::PG_KEYWORD_UNRESERVED:
        res.category_ = KeywordCategory::KEYWORD_UNRESERVED;
        break;
      case duckdb_libpgquery::PGKeywordCategory::PG_KEYWORD_TYPE_FUNC:
        res.category_ = KeywordCategory::KEYWORD_TYPE_FUNC;
        break;
      case duckdb_libpgquery::PGKeywordCategory::PG_KEYWORD_COL_NAME:
        res.category_ = KeywordCategory::KEYWORD_COL_NAME;
        break;
      default:
        throw Exception("Unrecognized keyword category");
    }
    result.push_back(res);
  }
  return result;
}

auto Binder::Tokenize(const std::string &query) -> std::vector<SimplifiedToken> {
  auto pg_tokens = duckdb::PostgresParser::Tokenize(query);
  std::vector<SimplifiedToken> result;
  result.reserve(pg_tokens.size());
  for (auto &pg_token : pg_tokens) {
    SimplifiedToken token;
    switch (pg_token.type) {
      case duckdb_libpgquery::PGSimplifiedTokenType::PG_SIMPLIFIED_TOKEN_IDENTIFIER:
        token.type_ = SimplifiedTokenType::SIMPLIFIED_TOKEN_IDENTIFIER;
        break;
      case duckdb_libpgquery::PGSimplifiedTokenType::PG_SIMPLIFIED_TOKEN_NUMERIC_CONSTANT:
        token.type_ = SimplifiedTokenType::SIMPLIFIED_TOKEN_NUMERIC_CONSTANT;
        break;
      case duckdb_libpgquery::PGSimplifiedTokenType::PG_SIMPLIFIED_TOKEN_STRING_CONSTANT:
        token.type_ = SimplifiedTokenType::SIMPLIFIED_TOKEN_STRING_CONSTANT;
        break;
      case duckdb_libpgquery::PGSimplifiedTokenType::PG_SIMPLIFIED_TOKEN_OPERATOR:
        token.type_ = SimplifiedTokenType::SIMPLIFIED_TOKEN_OPERATOR;
        break;
      case duckdb_libpgquery::PGSimplifiedTokenType::PG_SIMPLIFIED_TOKEN_KEYWORD:
        token.type_ = SimplifiedTokenType::SIMPLIFIED_TOKEN_KEYWORD;
        break;
      // comments are not supported by our tokenizer right now
      case duckdb_libpgquery::PGSimplifiedTokenType::PG_SIMPLIFIED_TOKEN_COMMENT:
        token.type_ = SimplifiedTokenType::SIMPLIFIED_TOKEN_COMMENT;
        break;
      default:
        throw Exception("Unrecognized token category");
    }
    token.start_ = pg_token.start;
    result.push_back(token);
  }
  return result;
}

auto Binder::BindCreateView(duckdb_libpgquery::PGViewStmt *pg_stmt) -> std::unique_ptr<CreateViewStatement> {
  // Validate the view name and query
  if (pg_stmt->view == nullptr || pg_stmt->query == nullptr) {
    throw Exception("View name or query cannot be empty");
  }

  // Extract the view name
  std::string view_name = pg_stmt->view->relname;

  // Convert the query node to a string representation
  std::string query = ConvertQueryNodeToString(pg_stmt->query);

  // Create and return the CreateViewStatement
  return std::make_unique<CreateViewStatement>(view_name, query);
}

std::string Binder::ConvertQueryNodeToString(duckdb_libpgquery::PGNode *node) {
  if (node == nullptr) {
    return "";
  }

  switch (node->type) {
    case duckdb_libpgquery::T_PGSelectStmt: {
      auto select_stmt = reinterpret_cast<duckdb_libpgquery::PGSelectStmt *>(node);
      std::string result = "SELECT ";
      if (select_stmt->distinctClause != nullptr) {
        result += "DISTINCT ";
      }
      result += ConvertQueryNodeToString(reinterpret_cast<duckdb_libpgquery::PGNode *>(select_stmt->targetList));
      result += " FROM ";
      result += ConvertQueryNodeToString(reinterpret_cast<duckdb_libpgquery::PGNode *>(select_stmt->fromClause));
      if (select_stmt->whereClause != nullptr) {
        result += " WHERE ";
        result += ConvertQueryNodeToString(select_stmt->whereClause);
      }
      if (select_stmt->groupClause != nullptr) {
        result += " GROUP BY ";
        result += ConvertQueryNodeToString(reinterpret_cast<duckdb_libpgquery::PGNode *>(select_stmt->groupClause));
      }
      if (select_stmt->havingClause != nullptr) {
        result += " HAVING ";
        result += ConvertQueryNodeToString(select_stmt->havingClause);
      }
      if (select_stmt->sortClause != nullptr) {
        result += " ORDER BY ";
        result += ConvertQueryNodeToString(reinterpret_cast<duckdb_libpgquery::PGNode *>(select_stmt->sortClause));
      }
      if (select_stmt->limitCount != nullptr) {
        result += " LIMIT ";
        result += ConvertQueryNodeToString(select_stmt->limitCount);
      }
      if (select_stmt->limitOffset != nullptr) {
        result += " OFFSET ";
        result += ConvertQueryNodeToString(select_stmt->limitOffset);
      }
      return result;
    }
    case duckdb_libpgquery::T_PGResTarget: {
      auto res_target = reinterpret_cast<duckdb_libpgquery::PGResTarget *>(node);
      return ConvertQueryNodeToString(res_target->val);
    }
    case duckdb_libpgquery::T_PGColumnRef: {
      auto column_ref = reinterpret_cast<duckdb_libpgquery::PGColumnRef *>(node);
      std::string result;
      for (auto cell = column_ref->fields->head; cell != nullptr; cell = cell->next) {
        if (!result.empty()) {
          result += ".";
        }
        auto field = reinterpret_cast<duckdb_libpgquery::PGNode *>(cell->data.ptr_value);
        if (field->type == duckdb_libpgquery::T_PGString) {
          result += reinterpret_cast<duckdb_libpgquery::PGValue *>(field)->val.str;
        }
      }
      return result;
    }
    case duckdb_libpgquery::T_PGAConst: {
      auto aconst = reinterpret_cast<duckdb_libpgquery::PGAConst *>(node);
      auto val = aconst->val;
      switch (val.type) {
        case duckdb_libpgquery::T_PGInteger:
          return std::to_string(val.val.ival);
        case duckdb_libpgquery::T_PGString:
          return fmt::format("'{}'", val.val.str);
        default:
          throw Exception("Unsupported constant type");
      }
    }
    case duckdb_libpgquery::T_PGAExpr: {
      auto aexpr = reinterpret_cast<duckdb_libpgquery::PGAExpr *>(node);
      std::string result = ConvertQueryNodeToString(aexpr->lexpr);
      result += " ";
      result += reinterpret_cast<duckdb_libpgquery::PGValue *>(aexpr->name->head->data.ptr_value)->val.str;
      result += " ";
      result += ConvertQueryNodeToString(aexpr->rexpr);
      return result;
    }
    case duckdb_libpgquery::T_PGFuncCall: {
      auto func_call = reinterpret_cast<duckdb_libpgquery::PGFuncCall *>(node);
      std::string result;
      for (auto cell = func_call->funcname->head; cell != nullptr; cell = cell->next) {
        if (!result.empty()) {
          result += ".";
        }
        result += reinterpret_cast<duckdb_libpgquery::PGValue *>(cell->data.ptr_value)->val.str;
      }
      result += "(";
      for (auto cell = func_call->args->head; cell != nullptr; cell = cell->next) {
        if (cell != func_call->args->head) {
          result += ", ";
        }
        result += ConvertQueryNodeToString(reinterpret_cast<duckdb_libpgquery::PGNode *>(cell->data.ptr_value));
      }
      result += ")";
      return result;
    }
    case duckdb_libpgquery::T_PGList: {
      std::string result;
      for (auto cell = reinterpret_cast<duckdb_libpgquery::PGList *>(node)->head; cell != nullptr; cell = cell->next) {
        if (!result.empty()) {
          result += ", ";
        }
        result += ConvertQueryNodeToString(reinterpret_cast<duckdb_libpgquery::PGNode *>(cell->data.ptr_value));
      }
      return result;
    }
    case duckdb_libpgquery::T_PGRangeVar: {
      auto range_var = reinterpret_cast<duckdb_libpgquery::PGRangeVar *>(node);
      std::string result;
      if (range_var->schemaname != nullptr) {
        result += range_var->schemaname;
        result += ".";
      }
      result += range_var->relname;
      return result;
    }
    default:
      throw Exception(fmt::format("Unsupported node type: {}", Binder::NodeTagToString(node->type)));
  }
}

auto Binder::BindRangeVar(duckdb_libpgquery::PGRangeVar *range_var) -> std::unique_ptr<BoundTableRef> {
  // Check if the range variable refers to a view
  auto view_query = catalog_.GetView(range_var->relname);
  if (!view_query.empty()) {
    // Expand the view into its underlying query
    auto view_stmt = reinterpret_cast<duckdb_libpgquery::PGSelectStmt*>(Parse(view_query));
    auto select_stmt = BindSelect(view_stmt);
    std::vector<std::vector<std::string>> select_list_name; // Placeholder for select list names
    return std::make_unique<BoundSubqueryRef>(std::move(select_stmt), std::move(select_list_name), range_var->relname);
  }

  // Otherwise, bind the table reference as usual
  auto table_info = catalog_.GetTable(range_var->relname);
  if (table_info == nullptr) {
    throw Exception(fmt::format("invalid table {}", range_var->relname));
  }

  return std::make_unique<BoundBaseTableRef>(table_info);
}

}  // namespace hmssql