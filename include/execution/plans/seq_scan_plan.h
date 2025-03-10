//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// seq_scan_plan.h
//
// Identification: src/include/execution/plans/seq_scan_plan.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/catalog/catalog.h"
#include "../include/catalog/schema.h"
#include "../include/execution/expressions/abstract_expression.h"
#include "../include/execution/plans/abstract_plan.h"

namespace hmssql {

/**
 * The SeqScanPlanNode represents a sequential table scan operation.
 */
class SeqScanPlanNode : public AbstractPlanNode {
 public:
  /**
   * Construct a new SeqScanPlanNode instance.
   * @param output The output schema of this sequential scan plan node
   * @param table_oid The identifier of table to be scanned
   */
  SeqScanPlanNode(SchemaRef output, table_oid_t table_oid, std::string table_name,
                  AbstractExpressionRef filter_predicate = nullptr)
      : AbstractPlanNode(std::move(output), {}),
        table_oid_{table_oid},
        table_name_(std::move(table_name)),
        filter_predicate_(std::move(filter_predicate)) {}

  /** @return The type of the plan node */
  auto GetType() const -> PlanType override { return PlanType::SeqScan; }

  /** @return The identifier of the table that should be scanned */
  auto GetTableOid() const -> table_oid_t { return table_oid_; }

  static auto InferScanSchema(const BoundBaseTableRef &table_ref) -> Schema;

  BUSTUB_PLAN_NODE_CLONE_WITH_CHILDREN(SeqScanPlanNode);

  /** The table whose tuples should be scanned */
  table_oid_t table_oid_;

  /** The table name */
  std::string table_name_;

  /** The predicate to filter in seqscan. It will ALWAYS be nullptr until you enable the MergeFilterScan rule.
      You don't need to handle it to get a perfect score as of in Fall 2022.
  */
  AbstractExpressionRef filter_predicate_;

 protected:
  auto PlanNodeToString() const -> std::string override {
    if (filter_predicate_) {
      return fmt::format("SeqScan {{ table={}, filter={} }}", table_name_, filter_predicate_);
    }
    return fmt::format("SeqScan {{ table={} }}", table_name_);
  }
};

}  // namespace hmssql
