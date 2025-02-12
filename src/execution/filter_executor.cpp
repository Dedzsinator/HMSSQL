// filepath: /c:/Users/degin/Desktop/bustubb/src/execution/filter_executor.cpp
#include "../include/execution/executors/filter_executor.h"
#include "../include/common/exception.h"
#include "../include/type/value_factory.h"
#include "../include/execution/expressions/like_expression.h"

namespace hmssql {

FilterExecutor::FilterExecutor(ExecutorContext *exec_ctx, const FilterPlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_(plan), child_executor_(std::move(child_executor)) {}

void FilterExecutor::Init() {
  // Initialize the child executor
  child_executor_->Init();
}

auto FilterExecutor::Next(Tuple *tuple, RID *rid) -> bool {
  auto filter_expr = plan_->GetPredicate();

  while (true) {
    // Get the next tuple
    const auto status = child_executor_->Next(tuple, rid);

    if (!status) {
      return false;
    }

    auto value = filter_expr->Evaluate(tuple, child_executor_->GetOutputSchema());
    if (!value.IsNull() && value.GetAs<bool>()) {
      return true;
    }
  }
}

}  // namespace hmssql