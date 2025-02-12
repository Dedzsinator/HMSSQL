#include <memory>
#include <utility>

#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_statement.h"
#include "../include/binder/bound_table_ref.h"
#include "../include/binder/statement/delete_statement.h"
#include "../include/binder/statement/insert_statement.h"
#include "../include/binder/statement/select_statement.h"
#include "../include/binder/statement/update_statement.h"
#include "../include/binder/tokens.h"
#include "../include/common/enums/statement_type.h"
#include "../include/common/exception.h"
#include "../include/common/macros.h"
#include "../include/common/util/string_util.h"
#include "../include/execution/expressions/abstract_expression.h"
#include "../include/execution/expressions/column_value_expression.h"
#include "../include/execution/plans/abstract_plan.h"
#include "../include/execution/plans/filter_plan.h"
#include "../include/execution/plans/projection_plan.h"
#include "fmt/format.h"
#include "../include/planner/planner.h"

namespace bustub {

void Planner::PlanQuery(const BoundStatement &statement) {
  switch (statement.type_) {
    case StatementType::SELECT_STATEMENT: {
      plan_ = PlanSelect(dynamic_cast<const SelectStatement &>(statement));
      return;
    }
    case StatementType::INSERT_STATEMENT: {
      plan_ = PlanInsert(dynamic_cast<const InsertStatement &>(statement));
      return;
    }
    case StatementType::DELETE_STATEMENT: {
      plan_ = PlanDelete(dynamic_cast<const DeleteStatement &>(statement));
      return;
    }
    case StatementType::UPDATE_STATEMENT: {
      plan_ = PlanUpdate(dynamic_cast<const UpdateStatement &>(statement));
      return;
    }
    default:
      throw Exception(fmt::format("the statement {} is not supported in planner yet", statement.type_));
  }
}

auto Planner::MakeOutputSchema(const std::vector<std::pair<std::string, TypeId>> &exprs) -> SchemaRef {
  std::vector<Column> cols;
  cols.reserve(exprs.size());
  for (const auto &[col_name, type_id] : exprs) {
    if (type_id != TypeId::VARCHAR) {
      cols.emplace_back(col_name, type_id);
    } else {
      cols.emplace_back(col_name, type_id, VARCHAR_DEFAULT_LENGTH);
    }
  }
  return std::make_shared<Schema>(cols);
}

void PlannerContext::AddAggregation(std::unique_ptr<BoundExpression> expr) {
  if (!allow_aggregation_) {
    throw bustub::Exception("AggCall not allowed in this position");
  }
  aggregations_.push_back(std::move(expr));
}

}  // namespace bustub
