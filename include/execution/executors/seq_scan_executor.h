//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// seq_scan_executor.h
//
// Identification: src/include/execution/executors/seq_scan_executor.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

#include "../include/execution/executor_context.h"
#include "../include/execution/executors/abstract_executor.h"
#include "../include/execution/plans/seq_scan_plan.h"
#include "../include/storage/table/tuple.h"

namespace hmssql {

/**
 * The SeqScanExecutor executor executes a sequential table scan.
 */
class SeqScanExecutor : public AbstractExecutor {
 public:
  /**
   * Construct a new SeqScanExecutor instance.
   * @param exec_ctx The executor context
   * @param plan The sequential scan plan to be executed
   */
  SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan);

  /** Initialize the sequential scan */
  void Init() override;

  /**
   * Yield the next tuple from the sequential scan.
   * @param[out] tuple The next tuple produced by the scan
   * @param[out] rid The next tuple RID produced by the scan
   * @return `true` if a tuple was produced, `false` if there are no more tuples
   */
  auto Next(Tuple *tuple, RID *rid) -> bool override;

  /** @return The output schema for the sequential scan */
  auto GetOutputSchema() const -> const Schema & override { return plan_->OutputSchema(); }

 private:
  /** The sequential scan plan node to be executed */
  const SeqScanPlanNode *plan_;
  TableIterator table_iter_ = {nullptr, RID()};
  const TableInfo *table_info_;
};
}  // namespace hmssql
