#include "../include/execution/execution_engine.h"
#include "../include/binder/bound_statement.h"

namespace hmssql {

void ExecutionEngine::ExecuteCreateView(const CreateViewStatement &stmt) {
  auto txn = GetTransaction();
  if (!catalog_->CreateView(txn, stmt.view_, stmt.query_)) {
    throw Exception("Failed to create view");
  }
}

void ExecutionEngine::ExecuteCreateTempTable(const CreateTempTableStatement &stmt) {
  auto txn = GetTransaction();
  auto schema = Schema(stmt.columns_);
  auto info = catalog_->CreateTempTable(txn, stmt.table_, schema);
  if (info == nullptr) {
    throw Exception("Failed to create temporary table");
  }
}

}  // namespace hmssql