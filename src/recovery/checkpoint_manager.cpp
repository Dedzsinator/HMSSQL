//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// checkpoint_manager.cpp
//
// Identification: src/recovery/checkpoint_manager.cpp
//
//
//===----------------------------------------------------------------------===//

#include "../include/recovery/checkpoint_manager.h"
#include "../third_party/spdlog/spdlog.h"

namespace hmssql {

void CheckpointManager::BeginCheckpoint() {
  std::unique_lock<std::mutex> lock(checkpoint_mutex_);
  
  // Check if checkpoint is already in progress
  if (checkpoint_in_progress_) {
    throw Exception("Checkpoint already in progress");
  }
  
  // Set checkpoint flag
  checkpoint_in_progress_ = true;
  
  // Flush WAL
  if (log_manager_ != nullptr) {
    log_manager_->FlushAllLogs();
  }
  
  // Flush all dirty pages to disk
  if (buffer_pool_manager_ != nullptr) {
    buffer_pool_manager_->FlushAllPages();
  }
}

void CheckpointManager::EndCheckpoint() {
  std::unique_lock<std::mutex> lock(checkpoint_mutex_);
  
  if (!checkpoint_in_progress_) {
    throw Exception("No checkpoint in progress");
  }
  
  // Resume WAL
  if (log_manager_ != nullptr) {
    log_manager_->RunFlushThread();
  }
  
  // Reset checkpoint flag
  checkpoint_in_progress_ = false;
  
  // Notify waiting threads
  checkpoint_cv_.notify_all();
  
  spdlog::info("Checkpoint completed - transactions resumed");
}

}  // namespace hmssql