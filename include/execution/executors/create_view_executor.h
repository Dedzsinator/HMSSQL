#pragma once

#include "abstract_executor.h"
#include "execution/plans/create_view_plan.h"
#include "catalog/catalog.h"

namespace hmssql {

class CreateViewExecutor : public AbstractExecutor {
 public:
  CreateViewExecutor(ExecutorContext *exec_ctx, const CreateViewPlanNode *plan)
      : AbstractExecutor(exec_ctx), plan_(plan) {}

  void Init() override {
    // Initialize the executor
  }

  bool Next(Tuple *tuple, RID *rid) override {
    // Execute the create view plan
    auto catalog = exec_ctx_->GetCatalog();
    catalog->CreateView(plan_->GetViewName(), plan_->GetQuery());
    return false;
  }

  auto GetOutputSchema() const -> const Schema & override {
    return plan_->OutputSchema();
  }

 private:
  const CreateViewPlanNode *plan_;
};

}  // namespace hmssql