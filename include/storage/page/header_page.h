//===----------------------------------------------------------------------===//
//
// Identification: src/include/page/header_page.h
//
//===----------------------------------------------------------------------===//
#pragma once

#include <cstring>
#include <string>
#include "../include/storage/page/page.h"

namespace hmssql {

/**
 * Database use the first page (page_id = 0) as header page to store metadata, in
 * our case, we will contain information about table/index name (length less than
 * 32 bytes) and their corresponding root_id
 *
 * Format (size in byte):
 *  -----------------------------------------------------------------
 * | RecordCount (4) | Entry_1 name (32) | Entry_1 root_id (4) | ... |
 *  -----------------------------------------------------------------
 */
class HeaderPage : public Page {
 public:
  void Init() { SetRecordCount(0); }
  /**
   * Record related
   */
  auto InsertRecord(const std::string &name, page_id_t root_id) -> bool;
  auto DeleteRecord(const std::string &name) -> bool;
  auto UpdateRecord(const std::string &name, page_id_t root_id) -> bool;

  // return root_id if success
  auto GetRootId(const std::string &name, page_id_t *root_id) -> bool;
  auto GetRecordCount() -> int;

 private:
  /**
   * helper functions
   */
  auto FindRecord(const std::string &name) -> int;

  void SetRecordCount(int record_count);
};
}  // namespace hmssql
