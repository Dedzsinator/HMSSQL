#include <iterator>
#include <memory>
#include <optional>
#include <string>

#include "../include/binder/binder.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_order_by.h"
#include "../include/binder/bound_table_ref.h"
#include "../include/binder/bound_statement.h"
#include "../include/binder/tokens.h"
#include "../include/common/exception.h"
#include "../include/common/util/string_util.h"
#include "nodes/parsenodes.hpp"
#include "../include/type/value_factory.h"

namespace hmssql {

auto Binder::BindInsert(duckdb_libpgquery::PGInsertStmt *pg_stmt) -> std::unique_ptr<InsertStatement> {
  if (pg_stmt->cols != nullptr) {
    throw NotImplementedException("insert only supports all columns, don't specify columns");
  }

  auto table = BindBaseTableRef(pg_stmt->relation->relname, {});

  if (StringUtil::StartsWith(table->table_, "__")) {
    throw hmssql::Exception(fmt::format("invalid table for insert: {}", table->table_));
  }

  // Save current scope
  const BoundTableRef* current_scope = scope_;
  
  // Set the table as the current scope for binding expressions
  scope_ = table.get();
  
  auto select_statement = BindSelect(reinterpret_cast<duckdb_libpgquery::PGSelectStmt *>(pg_stmt->selectStmt));
  
  // Restore original scope
  scope_ = current_scope;

  return std::make_unique<InsertStatement>(std::move(table), std::move(select_statement));
}

auto Binder::BindDelete(duckdb_libpgquery::PGDeleteStmt *stmt) -> std::unique_ptr<DeleteStatement> {
  auto table = BindBaseTableRef(stmt->relation->relname, std::nullopt);
  auto ctx_guard = NewContext();
  scope_ = table.get();
  std::unique_ptr<BoundExpression> expr = nullptr;
  if (stmt->whereClause != nullptr) {
    expr = BindExpression(stmt->whereClause);
  } else {
    expr = std::make_unique<BoundConstant>(ValueFactory::GetBooleanValue(true));
  }

  return std::make_unique<DeleteStatement>(std::move(table), std::move(expr));
}

auto Binder::BindUpdate(duckdb_libpgquery::PGUpdateStmt *stmt) -> std::unique_ptr<UpdateStatement> {
  if (stmt->withClause != nullptr) {
    throw hmssql::NotImplementedException("update with clause not supported yet");
  }

  if (stmt->fromClause != nullptr) {
    throw hmssql::NotImplementedException("update from clause not supported yet");
  }

  auto table = BindBaseTableRef(stmt->relation->relname, std::nullopt);
  auto ctx_guard = NewContext();
  scope_ = table.get();

  std::unique_ptr<BoundExpression> filter_expr = nullptr;

  if (stmt->whereClause != nullptr) {
    filter_expr = BindExpression(stmt->whereClause);
  } else {
    filter_expr = std::make_unique<BoundConstant>(ValueFactory::GetBooleanValue(true));
  }

  auto root = stmt->targetList;
  std::vector<std::pair<std::unique_ptr<BoundColumnRef>, std::unique_ptr<BoundExpression>>> target_expr;

  for (auto cell = root->head; cell != nullptr; cell = cell->next) {
    auto target = reinterpret_cast<duckdb_libpgquery::PGResTarget *>(cell->data.ptr_value);
    auto column = ResolveColumnRefFromBaseTableRef(*table, std::vector{std::string{target->name}});
    target_expr.emplace_back(std::make_pair(std::move(column), BindExpression(target->val)));
  }

  return std::make_unique<UpdateStatement>(std::move(table), std::move(filter_expr), std::move(target_expr));
}

}  // namespace hmssql
