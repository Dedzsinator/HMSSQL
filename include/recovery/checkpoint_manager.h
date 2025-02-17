//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// checkpoint_manager.h
//
// Identification: src/include/recovery/checkpoint_manager.h
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include <mutex>
#include <condition_variable>
#include "../buffer/buffer_pool_manager.h"
#include "../concurrency/transaction_manager.h"
#include "../recovery/log_manager.h"

namespace hmssql {

class CheckpointManager {
 public:
  CheckpointManager(TransactionManager *transaction_manager, LogManager *log_manager,
                   BufferPoolManager *buffer_pool_manager)
      : transaction_manager_(transaction_manager),
        log_manager_(log_manager),
        buffer_pool_manager_(buffer_pool_manager),
        checkpoint_in_progress_(false) {}

  ~CheckpointManager() = default;
  void BeginCheckpoint();
  void EndCheckpoint();

 private:
  TransactionManager *transaction_manager_;
  LogManager *log_manager_;
  BufferPoolManager *buffer_pool_manager_;
  
  // Checkpoint state
  std::mutex checkpoint_mutex_;
  std::condition_variable checkpoint_cv_;
  bool checkpoint_in_progress_;
};

}  // namespace hmssql