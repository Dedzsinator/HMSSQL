#pragma once

#include "abstract_executor.h"
#include "execution/plans/create_temptable_plan.h"
#include "catalog/catalog.h"

namespace hmssql {

class CreateTempTableExecutor : public AbstractExecutor {
 public:
  CreateTempTableExecutor(ExecutorContext *exec_ctx, const CreateTempTablePlanNode *plan)
      : AbstractExecutor(exec_ctx), plan_(plan) {}

  void Init() override {
    // Initialize the executor
  }

  bool Next(Tuple *tuple, RID *rid) override {
    // Execute the create temp table plan
    auto catalog = exec_ctx_->GetCatalog();
    Schema schema(plan_->GetColumns());
    catalog->CreateTempTable(plan_->GetTableName(), schema);
    return false;
  }

  auto GetOutputSchema() const -> const Schema & override {
    return plan_->OutputSchema();
  }

 private:
  const CreateTempTablePlanNode *plan_;
};

}  // namespace hmssql