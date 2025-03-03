//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// logger.h
//
// Identification: include/common/logger.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"

namespace hmssql {

/**
 * Logger levels:
 * - trace (lowest): very detailed logs for tracing code execution
 * - debug: detailed debugging information
 * - info (blue): general information about regular operations
 * - warn (yellow): warnings that need attention
 * - error (red): errors that impacted operations
 * - critical (bright red): critical errors that may cause system failure
 */
enum class LogLevel {
  TRACE = spdlog::level::trace,
  DEBUG = spdlog::level::debug,
  INFO = spdlog::level::info,
  SUCCESS = spdlog::level::info,  // Use info level for success messages
  WARN = spdlog::level::warn,
  ERROR = spdlog::level::err,
  CRITICAL = spdlog::level::critical,
  OFF = spdlog::level::off
};

/**
 * Logger manager for HMSSQL database system
 */
class Logger {
 public:
  static Logger &GetInstance();
  
  /**
   * Initialize the logging system
   * 
   * @param enable_console Enable console logging
   * @param log_directory Directory for log files
   * @param level Minimum logging level
   */
  void Initialize(bool enable_console = true, 
                 const std::string &log_directory = "logs",
                 LogLevel level = LogLevel::INFO);

  /**
   * Set the global logging level
   */
  void SetLevel(LogLevel level);

  /**
   * Get or create a logger for a specific module
   */
  std::shared_ptr<spdlog::logger> GetLogger(const std::string &name);

  /**
   * Register a custom sink with all loggers
   */
  void AddSink(const spdlog::sink_ptr &sink);

 private:
  Logger() = default;
  ~Logger() = default;
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;
  
  bool initialized_ = false;
  
  // Default sinks used by all loggers
  std::vector<spdlog::sink_ptr> sinks_;
  
  // Cache of created loggers
  std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> loggers_;
  
  // Register custom log levels
  void RegisterCustomLogLevels();
  
  // Configure spdlog pattern with color
  void ConfigurePattern();
};

// Convenience global functions
inline std::shared_ptr<spdlog::logger> GetLogger(const std::string &name) {
  return Logger::GetInstance().GetLogger(name);
}

// Define macro wrappers for logging
#define LOG_TRACE(logger, ...) SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__)
#define LOG_DEBUG(logger, ...) SPDLOG_LOGGER_DEBUG(logger, __VA_ARGS__)
#define LOG_INFO(logger, ...) SPDLOG_LOGGER_INFO(logger, __VA_ARGS__)
// Use info level with special success prefix for success messages
#define LOG_SUCCESS(logger, ...) logger->info("[SUCCESS] " __VA_ARGS__)
#define LOG_WARN(logger, ...) SPDLOG_LOGGER_WARN(logger, __VA_ARGS__)
#define LOG_ERROR(logger, ...) SPDLOG_LOGGER_ERROR(logger, __VA_ARGS__)
#define LOG_CRITICAL(logger, ...) SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__)

} // namespace hmssql