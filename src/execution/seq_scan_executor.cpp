//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// seq_scan_executor.cpp
//
// Identification: src/execution/seq_scan_executor.cpp
//
//
//===----------------------------------------------------------------------===//

#include "../include/execution/executors/seq_scan_executor.h"

namespace hmssql {

SeqScanExecutor::SeqScanExecutor(ExecutorContext *exec_ctx, const SeqScanPlanNode *plan)
    : AbstractExecutor(exec_ctx), plan_(plan) {
  this->table_info_ = this->exec_ctx_->GetCatalog()->GetTable(plan_->table_oid_);
}

void SeqScanExecutor::Init() {
  this->table_iter_ = table_info_->table_->Begin();
}

auto SeqScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  do {
    if (table_iter_ == table_info_->table_->End()) {
      return false;
    }
    *tuple = *table_iter_;
    *rid = tuple->GetRid();
    ++table_iter_;
  } while (plan_->filter_predicate_ != nullptr &&
          !plan_->filter_predicate_->Evaluate(tuple, table_info_->schema_).GetAs<bool>());

  return true;
}

}  // namespace hmssql
