//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// logger.cpp
//
// Identification: src/common/logger.cpp
//
//===----------------------------------------------------------------------===//

#include "../../include/common/logger.h"
#include <filesystem>
#include <iostream>

namespace hmssql {

Logger &Logger::GetInstance() {
  static Logger instance;
  return instance;
}

void Logger::Initialize(bool enable_console, const std::string &log_directory, LogLevel level) {
  if (initialized_) {
    return;
  }

  // Register our custom log levels
  RegisterCustomLogLevels();
  
  // Create log directory if it doesn't exist
  if (!log_directory.empty()) {
    try {
      std::filesystem::create_directories(log_directory);
    } catch (const std::exception &e) {
      std::cerr << "Failed to create log directory: " << e.what() << std::endl;
    }
  }

  // Add console sink if enabled
  if (enable_console) {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::level_enum(level));
    sinks_.push_back(console_sink);
  }

  // Add rotating file sink for all logs
  if (!log_directory.empty()) {
    auto main_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        log_directory + "/hmssql.log", 10 * 1024 * 1024, 10);
    main_file_sink->set_level(spdlog::level::trace); // Log everything to file
    sinks_.push_back(main_file_sink);
    
    // Add separate sinks for errors and above
    auto error_file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        log_directory + "/errors.log", 10 * 1024 * 1024, 10);
    error_file_sink->set_level(spdlog::level::err); // Only errors and above
    sinks_.push_back(error_file_sink);
  }

  // Create default logger
  auto default_logger = std::make_shared<spdlog::logger>("hmssql", sinks_.begin(), sinks_.end());
  default_logger->set_level(spdlog::level::level_enum(level));
  
  // Set as default and store it
  spdlog::set_default_logger(default_logger);
  loggers_["hmssql"] = default_logger;
  
  // Configure pattern with timestamps and thread IDs
  ConfigurePattern();

  // Set flush policy - flush on warning or higher
  spdlog::flush_on(spdlog::level::warn);

  initialized_ = true;
}

void Logger::SetLevel(LogLevel level) {
  spdlog::set_level(spdlog::level::level_enum(level));
  
  for (auto &sink_ptr : sinks_) {
    // Fix: Check sink type by dynamic_cast instead of using type()
    if (std::dynamic_pointer_cast<spdlog::sinks::stdout_color_sink_mt>(sink_ptr)) {
      // Keep console sink at the specified level
      sink_ptr->set_level(spdlog::level::level_enum(level));
    }
  }
}

std::shared_ptr<spdlog::logger> Logger::GetLogger(const std::string &name) {
  // Initialize with default settings if not yet initialized
  if (!initialized_) {
    Initialize();
  }
  
  // Return existing logger if we have one
  auto it = loggers_.find(name);
  if (it != loggers_.end()) {
    return it->second;
  }
  
  // Create a new logger with our shared sinks
  auto logger = std::make_shared<spdlog::logger>(name, sinks_.begin(), sinks_.end());
  
  // Set the same level as the global level
  logger->set_level(spdlog::get_level());
  
  // Store and return it
  loggers_[name] = logger;
  spdlog::register_logger(logger);
  return logger;
}

void Logger::AddSink(const spdlog::sink_ptr &sink) {
  sinks_.push_back(sink);
  
  // Add sink to all existing loggers
  for (auto &logger_entry : loggers_) {
    logger_entry.second->sinks().push_back(sink);
  }
}

void Logger::RegisterCustomLogLevels() {
  // Fix: Create custom level using spdlog API without register_level
  // We'll use existing levels and apply color formatting in the pattern
  // Instead of registering a new level, we'll use info level for success messages
  // and customize their appearance through the pattern
}

void Logger::ConfigurePattern() {
  // Set a pattern with color, timestamp, level, logger name, thread id, and message
  // %^...%$ is for colors
  // %Y-%m-%d %H:%M:%S.%e is the timestamp
  // [%l] is the level
  // %n is the logger name
  // (%t) is the thread id
  // %v is the actual message
  spdlog::set_pattern("%^[%Y-%m-%d %H:%M:%S.%e] [%l] [%n](%t)%$ %v");
}

} // namespace hmssql