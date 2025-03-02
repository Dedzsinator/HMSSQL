//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// index_scan_executor.cpp
//
// Identification: src/execution/index_scan_executor.cpp
//
//
//===----------------------------------------------------------------------===//
#include "../include/execution/executors/index_scan_executor.h"
#include "../include/execution/expressions/constant_value_expression.h"


namespace hmssql {
IndexScanExecutor::IndexScanExecutor(ExecutorContext *exec_ctx, const IndexScanPlanNode *plan)
    : AbstractExecutor(exec_ctx),
      plan_{plan},
      index_info_{this->exec_ctx_->GetCatalog()->GetIndex(plan_->index_oid_)},
      table_info_{this->exec_ctx_->GetCatalog()->GetTable(index_info_->table_name_)},
      tree_{dynamic_cast<BPlusTreeIndexForOneIntegerColumn *>(index_info_->index_.get())},
      iter_{plan_->filter_predicate_ != nullptr ? BPlusTreeIndexIteratorForOneIntegerColumn(nullptr, nullptr)
                                                : tree_->GetBeginIterator()} {}

void IndexScanExecutor::Init() {
  if (plan_->filter_predicate_ != nullptr) {
    const auto *right_expr =
        dynamic_cast<const ConstantValueExpression *>(plan_->filter_predicate_->children_[1].get());
    Value v = right_expr->val_;
    tree_->ScanKey(Tuple{{v}, index_info_->index_->GetKeySchema()}, &rids_);
    rid_iter_ = rids_.begin();
  }
}

auto IndexScanExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (plan_->filter_predicate_ != nullptr) {
    if (rid_iter_ != rids_.end()) {
      *rid = *rid_iter_;

      auto result = table_info_->table_->GetTuple(*rid, tuple);
      rid_iter_++;
      return result;
    }
    return false;
  }
  if (iter_ == tree_->GetEndIterator()) {
    return false;
  }
  *rid = (*iter_).second;
  auto result = table_info_->table_->GetTuple(*rid, tuple);
  ++iter_;

  return result;
}

}  // namespace hmssql