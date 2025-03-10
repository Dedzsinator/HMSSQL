//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// nested_index_join_executor.h
//
// Identification: src/include/execution/executors/nested_index_join_executor.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../include/execution/executor_context.h"
#include "../include/execution/executors/abstract_executor.h"
#include "../include/execution/expressions/abstract_expression.h"
#include "../include/execution/plans/nested_index_join_plan.h"
#include "../include/storage/table/tmp_tuple.h"
#include "../include/storage/table/tuple.h"

namespace hmssql {

/**
 * IndexJoinExecutor executes index join operations.
 */
class NestIndexJoinExecutor : public AbstractExecutor {
 public:
  /**
   * Creates a new nested index join executor.
   * @param exec_ctx the context that the hash join should be performed in
   * @param plan the nested index join plan node
   * @param child_executor the outer table
   */
  NestIndexJoinExecutor(ExecutorContext *exec_ctx, const NestedIndexJoinPlanNode *plan,
                        std::unique_ptr<AbstractExecutor> &&child_executor);

  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

  void Init() override;

  auto Next(Tuple *tuple, RID *rid) -> bool override;

 private:
  /** The nested index join plan node. */
  const NestedIndexJoinPlanNode *plan_;

  std::unique_ptr<AbstractExecutor> child_;
  const IndexInfo *index_info_;
  const TableInfo *table_info_;
  BPlusTreeIndexForOneIntegerColumn *tree_;
};
}  // namespace hmssql
