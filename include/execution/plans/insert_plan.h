//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// insert_plan.h
//
// Identification: src/include/execution/plans/insert_plan.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../include/catalog/catalog.h"
#include "../include/execution/expressions/abstract_expression.h"
#include "../include/execution/plans/abstract_plan.h"

namespace hmssql {

/**
 * The InsertPlanNode identifies a table into which tuples are inserted.
 *
 * The values to be inserted will come from the child of the node.
 */
class InsertPlanNode : public AbstractPlanNode {
 public:
  /**
   * Creates a new insert plan node for inserting values from a child plan.
   * @param child the child plan to obtain values from
   * @param table_oid the identifier of the table that should be inserted into
   */
  InsertPlanNode(SchemaRef output, AbstractPlanNodeRef child, table_oid_t table_oid)
      : AbstractPlanNode(std::move(output), {std::move(child)}), table_oid_(table_oid) {}

  /** @return The type of the plan node */
  auto GetType() const -> PlanType override { return PlanType::Insert; }

  /** @return The identifier of the table into which tuples are inserted */
  auto TableOid() const -> table_oid_t { return table_oid_; }

  /** @return the child plan providing tuples to be inserted */
  auto GetChildPlan() const -> AbstractPlanNodeRef {
    BUSTUB_ASSERT(GetChildren().size() == 1, "Insert should have only one child plan.");
    return GetChildAt(0);
  }

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(InsertPlanNode);

  /** The table to be inserted into. */
  table_oid_t table_oid_;

 protected:
  auto PlanNodeToString() const -> std::string override { return fmt::format("Insert {{ table_oid={} }}", table_oid_); }
};

}  // namespace hmssql
