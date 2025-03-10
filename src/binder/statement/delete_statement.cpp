#include "../include/binder/bound_statement.h"
#include "fmt/core.h"

namespace hmssql {

DeleteStatement::DeleteStatement(std::unique_ptr<BoundBaseTableRef> table, std::unique_ptr<BoundExpression> expr)
    : BoundStatement(StatementType::DELETE_STATEMENT), table_(std::move(table)), expr_(std::move(expr)) {}

auto DeleteStatement::ToString() const -> std::string {
  return fmt::format("Delete {{ table={}, expr={} }}", *table_, *expr_);
}

}  // namespace hmssql
