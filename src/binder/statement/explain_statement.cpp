
#include "fmt/ranges.h"
#include "../include/binder/binder.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_order_by.h"
#include "../include/binder/bound_table_ref.h"
#include "../include/binder/bound_statement.h"
#include "../include/common/util/string_util.h"

namespace hmssql {

ExplainStatement::ExplainStatement(std::unique_ptr<BoundStatement> statement, uint8_t options)
    : BoundStatement(StatementType::EXPLAIN_STATEMENT), statement_(std::move(statement)), options_(options) {}

auto ExplainStatement::ToString() const -> std::string {
  return fmt::format("BoundExplain {{\n  statement={},\n  options={},\n}}",
                     StringUtil::IndentAllLines(statement_->ToString(), 2, true), options_);
}

}  // namespace hmssql
