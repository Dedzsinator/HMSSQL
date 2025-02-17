//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// log_manager.h
//
// Identification: src/include/recovery/log_manager.h
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>
#include "log_record.h"
#include "../storage/disk/disk_manager.h"

namespace hmssql {

class LogManager {
public:
    explicit LogManager(DiskManager *disk_manager)
    : disk_manager_(disk_manager),
      persistent_lsn_(INVALID_LSN),
      next_lsn_(0),
      flush_thread_running_(false),
      flush_thread_(nullptr) {}

    ~LogManager() {
      StopFlushThread();
    }
    // Start background flush thread
    void RunFlushThread();
    
    // Stop and join the flush thread
    void StopFlushThread();
    
    // Flush all logs in buffer to disk
    void FlushAllLogs();
    
    // Append a new log record
    auto AppendLogRecord(LogRecord *log_record) -> lsn_t;
    
    // Getters
    auto GetNextLSN() -> lsn_t { return next_lsn_; }
    auto GetPersistentLSN() -> lsn_t { return persistent_lsn_; }

private:
    // Dependencies
    DiskManager *disk_manager_;
    
    // Thread safety components
    std::mutex latch_;
    std::condition_variable cv_;
    
    // LSN management
    std::atomic<lsn_t> persistent_lsn_;
    std::atomic<lsn_t> next_lsn_;
    
    // Thread management
    bool flush_thread_running_;
    std::thread *flush_thread_;

    // Log storage
    std::vector<LogRecord> log_buffer_;
};

} // namespace hmssql