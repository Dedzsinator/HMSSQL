//===----------------------------------------------------------------------===//
// Copyright 2018-2022 Stichting DuckDB Foundation
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice (including the next paragraph)
// shall be included in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//===----------------------------------------------------------------------===//

#include <memory>
#include "../include/binder/binder.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_order_by.h"
#include "../include/binder/bound_statement.h"
#include "../include/binder/statement/create_statement.h"
#include "../include/binder/statement/delete_statement.h"
#include "../include/binder/statement/explain_statement.h"
#include "../include/binder/statement/index_statement.h"
#include "../include/binder/statement/insert_statement.h"
#include "../include/binder/statement/select_statement.h"
#include "../include/binder/statement/update_statement.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/common/exception.h"
#include "../include/common/logger.h"
#include "../include/common/util/string_util.h"
#include "../include/nodes/nodes.hpp"
#include "../include/nodes/parsenodes.hpp"
#include "../include/type/decimal_type.h"
#include "../include/binder/statement/use_statement.h"
#include "../include/binder/statement/create_db_statement.h"

namespace hmssql {

void Binder::SaveParseTree(duckdb_libpgquery::PGList *tree) {
  std::vector<std::unique_ptr<BoundStatement>> statements;
  for (auto entry = tree->head; entry != nullptr; entry = entry->next) {
    statement_nodes_.push_back(reinterpret_cast<duckdb_libpgquery::PGNode *>(entry->data.ptr_value));
  }
}

auto Binder::BindStatement(duckdb_libpgquery::PGNode *stmt) -> std::unique_ptr<BoundStatement> {
  switch (stmt->type) {
    case duckdb_libpgquery::T_PGRawStmt:
      return BindStatement(reinterpret_cast<duckdb_libpgquery::PGRawStmt *>(stmt)->stmt);
    case duckdb_libpgquery::T_PGInsertStmt:
      return BindInsert(reinterpret_cast<duckdb_libpgquery::PGInsertStmt *>(stmt));
    case duckdb_libpgquery::T_PGSelectStmt:
      return BindSelect(reinterpret_cast<duckdb_libpgquery::PGSelectStmt *>(stmt));
    case duckdb_libpgquery::T_PGExplainStmt:
      return BindExplain(reinterpret_cast<duckdb_libpgquery::PGExplainStmt *>(stmt));
    case duckdb_libpgquery::T_PGDeleteStmt:
      return BindDelete(reinterpret_cast<duckdb_libpgquery::PGDeleteStmt *>(stmt));
    case duckdb_libpgquery::T_PGUpdateStmt:
      return BindUpdate(reinterpret_cast<duckdb_libpgquery::PGUpdateStmt *>(stmt));
    case duckdb_libpgquery::T_PGViewStmt:  // Add this case
      return BindCreateView(reinterpret_cast<duckdb_libpgquery::PGViewStmt *>(stmt));
    case duckdb_libpgquery::T_PGCreateStmt: {
      auto create_stmt = reinterpret_cast<duckdb_libpgquery::PGCreateStmt *>(stmt);
      // Check if this is a CREATE DATABASE command
      if (create_stmt->relation && create_stmt->relation->relpersistence == 'd') {
        std::string db_name = create_stmt->relation->relname;
        // Remove trailing semicolon if present
        if (!db_name.empty() && db_name.back() == ';') {
          db_name.pop_back();
        }
        return std::make_unique<CreateDatabaseStatement>(db_name);
      }
      return BindCreate(create_stmt);
    }

    case duckdb_libpgquery::T_PGCreateSchemaStmt: {
      // This handles our CREATE DATABASE which was converted to CREATE SCHEMA
      auto schema_stmt = reinterpret_cast<duckdb_libpgquery::PGCreateSchemaStmt *>(stmt);
      return std::make_unique<CreateDatabaseStatement>(schema_stmt->schemaname);
    }
    
    case duckdb_libpgquery::T_PGVariableSetStmt: {
      auto var_stmt = reinterpret_cast<duckdb_libpgquery::PGVariableSetStmt *>(stmt);
      if (var_stmt->name && StringUtil::Equals(var_stmt->name, "search_path")) {
        // This handles our USE statement which was converted to SET search_path
        auto val = reinterpret_cast<duckdb_libpgquery::PGAConst *>(var_stmt->args->head->data.ptr_value);
        std::string db_name = val->val.val.str;
        StringUtil::Trim(db_name);
        return std::make_unique<UseStatement>(db_name);
      }
      return BindVariableSet(var_stmt);
    }
    default:
      throw NotImplementedException(NodeTagToString(stmt->type));
  }
}

}  // namespace hmssql
