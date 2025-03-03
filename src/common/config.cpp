//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// config.cpp
//
// Identification: src/common/config.cpp
//
//
//===----------------------------------------------------------------------===//

#include "../include/common/config.h"
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>

namespace hmssql {

// For backward compatibility
std::atomic<bool> enable_logging(false);
std::chrono::milliseconds log_timeout = std::chrono::seconds(1);  // Changed type to milliseconds
std::chrono::milliseconds cycle_detection_interval = std::chrono::milliseconds(50);

Config::Config() {
  InitializeDefaults();
}

void Config::InitializeDefaults() {
  std::lock_guard<std::mutex> lock(config_mutex_);
  
  // System settings
  config_data_[ENABLE_LOGGING] = false;
  config_data_[LOG_TIMEOUT_MS] = 1000;  // 1 second
  config_data_[CYCLE_DETECTION_INTERVAL_MS] = 50;
  
  // Memory and storage settings
  config_data_[PAGE_SIZE] = BUSTUB_PAGE_SIZE;  // Use the global constant
  config_data_[BUFFER_POOL_SIZE] = 10;
  config_data_[LOG_BUFFER_SIZE] = ((10 + 1) * BUSTUB_PAGE_SIZE);  // Use the global constant
  config_data_[BUCKET_SIZE] = 50;
  config_data_[LRUK_REPLACER_K] = 10;
  
  // Schema settings
  config_data_[VARCHAR_DEFAULT_LENGTH] = 128;

  // Synchronize legacy globals with new config
  enable_logging.store(config_data_[ENABLE_LOGGING].get<bool>());
  log_timeout = std::chrono::milliseconds(config_data_[LOG_TIMEOUT_MS].get<int64_t>());
  cycle_detection_interval = std::chrono::milliseconds(config_data_[CYCLE_DETECTION_INTERVAL_MS].get<int64_t>());
}

bool Config::LoadFromFile(const std::string& config_file) {
  try {
    std::ifstream file(config_file);
    if (!file.is_open()) {
      return false;
    }
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    file >> config_data_;
    
    // Synchronize legacy globals with new config
    enable_logging.store(config_data_[ENABLE_LOGGING].get<bool>());
    log_timeout = std::chrono::milliseconds(config_data_[LOG_TIMEOUT_MS].get<int64_t>());
    cycle_detection_interval = std::chrono::milliseconds(config_data_[CYCLE_DETECTION_INTERVAL_MS].get<int64_t>());
    
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Error loading config file: " << e.what() << std::endl;
    return false;
  }
}

bool Config::SaveToFile(const std::string& config_file) {
  try {
    std::ofstream file(config_file);
    if (!file.is_open()) {
      return false;
    }
    
    std::lock_guard<std::mutex> lock(config_mutex_);
    file << config_data_.dump(4);  // Pretty print with 4-space indentation
    return true;
  } catch (const std::exception& e) {
    std::cerr << "Error saving config file: " << e.what() << std::endl;
    return false;
  }
}

int32_t Config::GetInt(const std::string& key, int32_t default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  if (config_data_.contains(key)) {
    return config_data_[key].get<int32_t>();
  }
  return default_value;
}

bool Config::GetBool(const std::string& key, bool default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  if (config_data_.contains(key)) {
    return config_data_[key].get<bool>();
  }
  return default_value;
}

std::string Config::GetString(const std::string& key, const std::string& default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  if (config_data_.contains(key)) {
    return config_data_[key].get<std::string>();
  }
  return default_value;
}

std::chrono::milliseconds Config::GetDuration(const std::string& key, int64_t default_value) const {
  std::lock_guard<std::mutex> lock(config_mutex_);
  if (config_data_.contains(key)) {
    return std::chrono::milliseconds(config_data_[key].get<int64_t>());
  }
  return std::chrono::milliseconds(default_value);
}

void Config::SetInt(const std::string& key, int32_t value) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_data_[key] = value;
}

void Config::SetBool(const std::string& key, bool value) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_data_[key] = value;
  
  // Update legacy globals
  if (key == ENABLE_LOGGING) {
    enable_logging.store(value);
  }
}

void Config::SetString(const std::string& key, const std::string& value) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_data_[key] = value;
}

void Config::SetDuration(const std::string& key, int64_t value) {
  std::lock_guard<std::mutex> lock(config_mutex_);
  config_data_[key] = value;
  
  // Update legacy globals
  if (key == LOG_TIMEOUT_MS) {
    log_timeout = std::chrono::milliseconds(value);
  } else if (key == CYCLE_DETECTION_INTERVAL_MS) {
    cycle_detection_interval = std::chrono::milliseconds(value);
  }
}

}  // namespace hmssql