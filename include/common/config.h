//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// config.h
//
// Identification: src/include/common/config.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <atomic>
#include <chrono>  // NOLINT
#include <cstdint>
#include <string>
#include <map>
#include <mutex>

#include "../../third_party/json/json.hpp"

namespace hmssql {

// Constants for backward compatibility (moved outside of class to be accessible everywhere)
constexpr int INVALID_PAGE_ID = -1;
constexpr int INVALID_TXN_ID = -1;
constexpr int INVALID_LSN = -1;
constexpr int HEADER_PAGE_ID = 0;
constexpr int BUSTUB_PAGE_SIZE = 4096;

// Additional constants needed by other parts of the codebase
constexpr int BUFFER_POOL_SIZE = 10;
constexpr int LOG_BUFFER_SIZE = (BUFFER_POOL_SIZE + 1) * BUSTUB_PAGE_SIZE;
constexpr int BUCKET_SIZE = 50;
constexpr int LRUK_REPLACER_K = 10; // Added this constant for backward compatibility
constexpr int VARCHAR_DEFAULT_LENGTH = 128;

// Type definitions (moved outside of class to be accessible everywhere)
using frame_id_t = int32_t;
using page_id_t = int32_t;
using txn_id_t = int32_t;
using lsn_t = int32_t;
using slot_offset_t = size_t;
using oid_t = uint16_t;

// For backward compatibility
extern std::atomic<bool> enable_logging;
extern std::chrono::milliseconds log_timeout;
extern std::chrono::milliseconds cycle_detection_interval;

/**
 * @brief Configuration manager for HMSSQL
 * 
 * This class manages configuration settings for the database system.
 * It uses a JSON-based approach to store and retrieve settings.
 */
class Config {
 public:
  /**
   * @brief Get the singleton instance of the Config class
   * 
   * @return Config& - The singleton instance
   */
  static Config& GetInstance() {
    static Config instance;
    return instance;
  }
  
  /**
   * @brief Initialize the configuration from a JSON file
   * 
   * @param config_file - Path to the configuration file
   * @return bool - True if successful, false otherwise
   */
  bool LoadFromFile(const std::string& config_file);
  
  /**
   * @brief Save the current configuration to a file
   * 
   * @param config_file - Path to the configuration file
   * @return bool - True if successful, false otherwise
   */
  bool SaveToFile(const std::string& config_file);
  
  /**
   * @brief Get an integer configuration value
   * 
   * @param key - Configuration key
   * @param default_value - Default value if key doesn't exist
   * @return int32_t - The configuration value
   */
  int32_t GetInt(const std::string& key, int32_t default_value = 0) const;
  
  /**
   * @brief Get a boolean configuration value
   * 
   * @param key - Configuration key
   * @param default_value - Default value if key doesn't exist
   * @return bool - The configuration value
   */
  bool GetBool(const std::string& key, bool default_value = false) const;
  
  /**
   * @brief Get a string configuration value
   * 
   * @param key - Configuration key
   * @param default_value - Default value if key doesn't exist
   * @return std::string - The configuration value
   */
  std::string GetString(const std::string& key, const std::string& default_value = "") const;
  
  /**
   * @brief Get a duration configuration value in milliseconds
   * 
   * @param key - Configuration key
   * @param default_value - Default value if key doesn't exist (in milliseconds)
   * @return std::chrono::milliseconds - The configuration value
   */
  std::chrono::milliseconds GetDuration(const std::string& key, int64_t default_value = 0) const;
  
  /**
   * @brief Set an integer configuration value
   * 
   * @param key - Configuration key
   * @param value - Value to set
   */
  void SetInt(const std::string& key, int32_t value);
  
  /**
   * @brief Set a boolean configuration value
   * 
   * @param key - Configuration key
   * @param value - Value to set
   */
  void SetBool(const std::string& key, bool value);
  
  /**
   * @brief Set a string configuration value
   * 
   * @param key - Configuration key
   * @param value - Value to set
   */
  void SetString(const std::string& key, const std::string& value);
  
  /**
   * @brief Set a duration configuration value
   * 
   * @param key - Configuration key
   * @param value - Value to set (in milliseconds)
   */
  void SetDuration(const std::string& key, int64_t value);

  // Constants for configuration keys
  static constexpr const char* ENABLE_LOGGING = "enable_logging";
  static constexpr const char* LOG_TIMEOUT_MS = "log_timeout_ms";
  static constexpr const char* CYCLE_DETECTION_INTERVAL_MS = "cycle_detection_interval_ms";
  static constexpr const char* PAGE_SIZE = "page_size";
  static constexpr const char* BUFFER_POOL_SIZE = "buffer_pool_size";
  static constexpr const char* LOG_BUFFER_SIZE = "log_buffer_size";
  static constexpr const char* BUCKET_SIZE = "bucket_size";
  static constexpr const char* LRUK_REPLACER_K = "lruk_replacer_k";
  static constexpr const char* VARCHAR_DEFAULT_LENGTH = "varchar_default_length";

 private:
  Config();
  ~Config() = default;
  
  // Delete copy and move constructors/assignments
  Config(const Config&) = delete;
  Config& operator=(const Config&) = delete;
  Config(Config&&) = delete;
  Config& operator=(Config&&) = delete;

  // Thread safety for configuration access
  mutable std::mutex config_mutex_;
  
  // The configuration data
  nlohmann::json config_data_;
  
  // Initialize with default values
  void InitializeDefaults();
};

// Helper functions to access global config
inline bool GetEnableLogging() {
  return Config::GetInstance().GetBool(Config::ENABLE_LOGGING);
}

inline void SetEnableLogging(bool value) {
  Config::GetInstance().SetBool(Config::ENABLE_LOGGING, value);
  enable_logging.store(value);
}

inline std::chrono::milliseconds GetLogTimeout() {
  return Config::GetInstance().GetDuration(Config::LOG_TIMEOUT_MS);
}

inline std::chrono::milliseconds GetCycleDetectionInterval() {
  return Config::GetInstance().GetDuration(Config::CYCLE_DETECTION_INTERVAL_MS);
}

inline int GetPageSize() {
  return Config::GetInstance().GetInt(Config::PAGE_SIZE);
}

inline int GetBufferPoolSize() {
  return Config::GetInstance().GetInt(Config::BUFFER_POOL_SIZE);
}

inline int GetLogBufferSize() {
  return Config::GetInstance().GetInt(Config::LOG_BUFFER_SIZE);
}

inline int GetBucketSize() {
  return Config::GetInstance().GetInt(Config::BUCKET_SIZE);
}

inline int GetLRUKReplacerK() {
  return Config::GetInstance().GetInt(Config::LRUK_REPLACER_K);
}

inline int GetVarcharDefaultLength() {
  return Config::GetInstance().GetInt(Config::VARCHAR_DEFAULT_LENGTH);
}

}  // namespace hmssql