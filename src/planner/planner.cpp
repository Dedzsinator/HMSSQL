#include <memory>
#include <utility>

#include "binder/bound_expression.h"
#include "binder/bound_statement.h"
#include "binder/bound_table_ref.h"
#include "binder/statement/delete_statement.h"
#include "binder/statement/insert_statement.h"
#include "binder/statement/select_statement.h"
#include "binder/statement/update_statement.h"
#include "binder/tokens.h"
#include "common/enums/statement_type.h"
#include "common/exception.h"
#include "common/macros.h"
#include "common/util/string_util.h"
#include "execution/expressions/abstract_expression.h"
#include "execution/expressions/column_value_expression.h"
#include "execution/plans/abstract_plan.h"
#include "execution/plans/filter_plan.h"
#include "execution/plans/projection_plan.h"
#include "fmt/format.h"
#include "planner/planner.h"
#include "execution/execution_engine.h"
#include "execution/plans/create_temptable_plan.h"
#include "execution/plans/create_view_plan.h"

namespace hmssql {

void Planner::PlanQuery(const BoundStatement &statement) {
  switch (statement.type_) {
    case StatementType::USE_STATEMENT: {
      // USE statements don't need planning since they're handled directly
      plan_ = nullptr;
      break;
    }
    
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
    case StatementType::CREATE_VIEW_STATEMENT: {
      plan_ = PlanCreateView(dynamic_cast<const CreateViewStatement &>(statement));
      return;
    }
    case StatementType::CREATE_TEMP_TABLE_STATEMENT: {
      plan_ = PlanCreateTempTable(dynamic_cast<const CreateTempTableStatement &>(statement));
      return;
    }
    default:
      throw Exception(fmt::format("the statement {} is not supported in planner yet", statement.type_));
  }
}

auto Planner::PlanCreateView(const CreateViewStatement &statement) -> AbstractPlanNodeRef {
  // Validate the view name and query
  if (statement.view_.empty() || statement.query_.empty()) {
    throw Exception("View name or query cannot be empty");
  }

  // Create a plan node for creating the view
  auto output_schema = std::make_shared<Schema>(std::vector<Column>{});
  return std::make_shared<CreateViewPlanNode>(output_schema, statement.view_, statement.query_);
}

auto Planner::PlanCreateTempTable(const CreateTempTableStatement &statement) -> AbstractPlanNodeRef {
  // Validate the table name and columns
  if (statement.table_.empty() || statement.columns_.empty()) {
    throw Exception("Table name or columns cannot be empty");
  }

  // Create a plan node for creating the temporary table
  auto output_schema = std::make_shared<Schema>(statement.columns_);
  return std::make_shared<CreateTempTablePlanNode>(output_schema, statement.table_, statement.columns_);
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
    throw hmssql::Exception("AggCall not allowed in this position");
  }
  aggregations_.push_back(std::move(expr));
}

}  // namespace hmssql
