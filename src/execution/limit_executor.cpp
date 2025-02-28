//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// limit_executor.cpp
//
// Identification: src/execution/limit_executor.cpp
//
//
//===----------------------------------------------------------------------===//

#include "../include/execution/executors/limit_executor.h"

namespace hmssql {

LimitExecutor::LimitExecutor(ExecutorContext *exec_ctx, const LimitPlanNode *plan,
                             std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_{plan}, child_executor_{std::move(child_executor)} {}

void LimitExecutor::Init() {
  child_executor_->Init();
  count_ = 0;
}

auto LimitExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  if (count_ >= plan_->GetLimit()) {
    return false;
  }

  if (!child_executor_->Next(tuple, rid)) {
    return false;
  }

  count_++;
  return true;
}

}  // namespace hmssql
