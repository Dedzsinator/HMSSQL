//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// log_manager.cpp
//
// Identification: src/recovery/log_manager.cpp
//
//
//===----------------------------------------------------------------------===//

#include "../include/recovery/log_manager.h"
#include "../third_party/spdlog/spdlog.h"

namespace hmssql {

void LogManager::RunFlushThread() {
    std::unique_lock<std::mutex> lock(latch_);
    if (flush_thread_running_) {
        return;
    }
    
    flush_thread_running_ = true;
    flush_thread_ = new std::thread([this] {
        while (flush_thread_running_) {
            std::unique_lock<std::mutex> guard(latch_);
            if (!log_buffer_.empty()) {
                FlushAllLogs();
            }
            cv_.wait_for(guard, std::chrono::milliseconds(100));
        }
    });
}

void LogManager::StopFlushThread() {
    {
        std::unique_lock<std::mutex> lock(latch_);
        if (!flush_thread_running_) {
            return;
        }
        flush_thread_running_ = false;
    }
    
    cv_.notify_one();
    if (flush_thread_ && flush_thread_->joinable()) {
        flush_thread_->join();
        delete flush_thread_;
        flush_thread_ = nullptr;
    }
}

void LogManager::FlushAllLogs() {
    std::unique_lock<std::mutex> lock(latch_);
    
    if (log_buffer_.empty()) {
        return;
    }

    for (const auto& record : log_buffer_) {
        // No need to remove const now since WriteLog accepts const char*
        disk_manager_->WriteLog(
            reinterpret_cast<const char*>(&record),
            sizeof(LogRecord)
        );
        // GetLSN() is now const-qualified
        persistent_lsn_ = record.GetLSN();
    }
    
    disk_manager_->FlushLog();
    log_buffer_.clear();
    
    spdlog::info("Flushed {} log records to disk", log_buffer_.size());
}

auto LogManager::AppendLogRecord(LogRecord *log_record) -> lsn_t {
    std::unique_lock<std::mutex> lock(latch_);
    
    log_record->lsn_ = next_lsn_++;
    log_buffer_.push_back(*log_record);
    
    return log_record->lsn_;
}

} // namespace hmssql