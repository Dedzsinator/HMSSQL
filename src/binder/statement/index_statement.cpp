#include "../include/binder/bound_statement.h"
#include "../include/binder/bound_expression.h"
#include "fmt/format.h"
#include "fmt/ranges.h"

namespace hmssql {

IndexStatement::IndexStatement(std::string index_name, std::unique_ptr<BoundBaseTableRef> table,
                               std::vector<std::unique_ptr<BoundColumnRef>> cols)
    : BoundStatement(StatementType::INDEX_STATEMENT),
      index_name_(std::move(index_name)),
      table_(std::move(table)),
      cols_(std::move(cols)) {}

auto IndexStatement::ToString() const -> std::string {
  return fmt::format("BoundIndex {{ index_name={}, table={}, cols={} }}", index_name_, *table_, cols_);
}

}  // namespace hmssql
