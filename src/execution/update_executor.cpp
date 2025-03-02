//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// update_executor.cpp
//
// Identification: src/execution/update_executor.cpp
//
//
//===----------------------------------------------------------------------===//
#include <memory>

#include "../include/execution/executors/update_executor.h"

namespace hmssql {

UpdateExecutor::UpdateExecutor(ExecutorContext *exec_ctx, const UpdatePlanNode *plan,
                              std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_{plan}, child_executor_{std::move(child_executor)} {
  // As of Fall 2022, you DON'T need to implement update executor to have perfect score in project 3 / project 4.
  this->table_info_ = this->exec_ctx_->GetCatalog()->GetTable(plan_->table_oid_);
}

void UpdateExecutor::Init() {
  child_executor_->Init();
  table_indexes_ = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
}

auto UpdateExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  if (is_end_) {
    return false;
  }
  Tuple old_tuple{};
  RID old_rid;
  int32_t update_count = 0;

  while (child_executor_->Next(&old_tuple, &old_rid)) {
    std::vector<Value> values{};
    values.reserve(child_executor_->GetOutputSchema().GetColumnCount());
    for (const auto &expr : plan_->target_expressions_) {
      values.push_back(expr->Evaluate(&old_tuple, child_executor_->GetOutputSchema()));
    }

    auto to_update_tuple = Tuple{values, &child_executor_->GetOutputSchema()};

    bool updated = table_info_->table_->UpdateTuple(to_update_tuple, old_rid);

    if (updated) {
      update_count++;
    }
  }
  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());
  values.emplace_back(TypeId::INTEGER, update_count);
  *tuple = Tuple{values, &GetOutputSchema()};
  is_end_ = true;
  return true;
}

}  // namespace hmssql
