
#include <memory>
#include "../include/binder/binder.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_statement.h"
#include "../include/common/exception.h"
namespace hmssql {

auto Binder::BindVariableSet(duckdb_libpgquery::PGVariableSetStmt *stmt) -> std::unique_ptr<VariableSetStatement> {
  auto expr = BindExpressionList(stmt->args);
  if (expr.size() != 1) {
    throw hmssql::NotImplementedException("Only exactly one arg is supported");
  }
  if (expr[0]->type_ != ExpressionType::CONSTANT) {
    throw hmssql::NotImplementedException("Only constant is supported");
  }
  const auto &const_expr = dynamic_cast<const BoundConstant &>(*expr[0]);
  return std::make_unique<VariableSetStatement>(stmt->name, const_expr.val_.ToString());
}

auto Binder::BindVariableShow(duckdb_libpgquery::PGVariableShowStmt *stmt) -> std::unique_ptr<VariableShowStatement> {
  return std::make_unique<VariableShowStatement>(stmt->name);
}

}  // namespace hmssql
