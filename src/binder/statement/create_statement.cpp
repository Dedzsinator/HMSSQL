#include "../include/catalog/column.h"
#include "fmt/ranges.h"

#include "../include/binder/statement/create_statement.h"

namespace hmssql {

CreateStatement::CreateStatement(std::string table, std::vector<Column> columns)
    : BoundStatement(StatementType::CREATE_STATEMENT), table_(std::move(table)), columns_(std::move(columns)) {}

auto CreateStatement::ToString() const -> std::string {
  return fmt::format("BoundCreate {{\n  table={}\n  columns={}\n}}", table_, columns_);
}

CreateViewStatement::CreateViewStatement(std::string view, std::string query)
    : BoundStatement(StatementType::CREATE_VIEW_STATEMENT), view_(std::move(view)), query_(std::move(query)) {}

auto CreateViewStatement::ToString() const -> std::string {
  return fmt::format("BoundCreateView {{\n  view={}\n  query={}\n}}", view_, query_);
}

CreateTempTableStatement::CreateTempTableStatement(std::string table, std::vector<Column> columns)
    : BoundStatement(StatementType::CREATE_TEMP_TABLE_STATEMENT), table_(std::move(table)), columns_(std::move(columns)) {}

auto CreateTempTableStatement::ToString() const -> std::string {
  return fmt::format("BoundCreateTempTable {{\n  table={}\n  columns={}\n}}", table_, columns_);
}

}  // namespace hmssql