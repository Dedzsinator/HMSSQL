//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// delete_executor.cpp
//
// Identification: src/execution/delete_executor.cpp
//
//
//===----------------------------------------------------------------------===//

#include <memory>

#include "../include/execution/executors/delete_executor.h"

namespace hmssql {

DeleteExecutor::DeleteExecutor(ExecutorContext *exec_ctx, const DeletePlanNode *plan,
                               std::unique_ptr<AbstractExecutor> &&child_executor)
    : AbstractExecutor(exec_ctx), plan_{plan}, child_executor_{std::move(child_executor)} {
  this->table_info_ = this->exec_ctx_->GetCatalog()->GetTable(plan_->table_oid_);
}

void DeleteExecutor::Init() {
  child_executor_->Init();
  // No transaction locking here anymore
  table_indexes_ = exec_ctx_->GetCatalog()->GetTableIndexes(table_info_->name_);
}

auto DeleteExecutor::Next([[maybe_unused]] Tuple *tuple, RID *rid) -> bool {
  if (is_end_) {
    return false;
  }
  Tuple to_delete_tuple{};
  RID emit_rid;
  int32_t delete_count = 0;

  while (child_executor_->Next(&to_delete_tuple, &emit_rid)) {
    // No transaction locking here anymore
    
    // Simply delete the tuple directly
    bool deleted = table_info_->table_->MarkDelete(emit_rid);

    if (deleted) {
      // Update indexes without transaction
      std::for_each(table_indexes_.begin(), table_indexes_.end(),
                    [&to_delete_tuple, &emit_rid, &table_info = table_info_](IndexInfo *index) {
                      index->index_->DeleteEntry(to_delete_tuple.KeyFromTuple(table_info->schema_, index->key_schema_,
                                                                              index->index_->GetKeyAttrs()), emit_rid);
                    });
      delete_count++;
    }
  }
  std::vector<Value> values{};
  values.reserve(GetOutputSchema().GetColumnCount());
  values.emplace_back(TypeId::INTEGER, delete_count);
  *tuple = Tuple{values, &GetOutputSchema()};
  is_end_ = true;
  return true;
}

}  // namespace hmssql