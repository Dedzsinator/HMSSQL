//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// hmssql_instance.h
//
// Identification: src/include/common/hmssql_instance.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../include/catalog/catalog.h"
#include "../include/common/config.h"
#include "../include/common/util/string_util.h"
#include "libfort/lib/fort.hpp"
#include "../include/type/value.h"

namespace hmssql {

class ExecutorContext;
class DiskManager;
class BufferPoolManager;
class LogManager;
class CheckpointManager;
class Catalog;
class ExecutionEngine;

class ResultWriter {
 public:
  ResultWriter() = default;
  virtual ~ResultWriter() = default;

  virtual void WriteCell(const std::string &cell) = 0;
  virtual void WriteHeaderCell(const std::string &cell) = 0;
  virtual void BeginHeader() = 0;
  virtual void EndHeader() = 0;
  virtual void BeginRow() = 0;
  virtual void EndRow() = 0;
  virtual void BeginTable(bool simplified_output) = 0;
  virtual void EndTable() = 0;

  bool simplified_output_{false};
};

class FortTableWriter : public ResultWriter {
 public:
  void WriteCell(const std::string &cell) override { table_ << cell; }
  void WriteHeaderCell(const std::string &cell) override { table_ << cell; }
  void BeginHeader() override { table_ << fort::header; }
  void EndHeader() override { table_ << fort::endr; }
  void BeginRow() override {}
  void EndRow() override { table_ << fort::endr; }
  void BeginTable(bool simplified_output) override {
    if (simplified_output) {
      table_.set_border_style(FT_EMPTY_STYLE);
    }
  }
  void EndTable() override {
    tables_.emplace_back(table_.to_string());
    table_ = fort::utf8_table{};
  }
  fort::utf8_table table_;
  std::vector<std::string> tables_;
};

class HMSSQL {
 private:
  /**
   * Get the executor context from the HMSSQL instance.
   */
  auto MakeExecutorContext() -> std::unique_ptr<ExecutorContext>;
  std::string current_database_;
  std::unordered_map<std::string, std::unique_ptr<Catalog>> databases_;
  std::shared_mutex databases_lock_;
  const std::string state_file_ = "hmssql_state.db";

 public:
  explicit HMSSQL(const std::string &db_file_name);

  HMSSQL();

  HMSSQL(const std::string& db_file, bool enable_logging);

  ~HMSSQL();

  auto SaveState() -> bool;
  auto LoadState() -> bool;
  auto Checkpoint() -> bool;  // Add semicolon here

  void CmdDisplayDatabases(ResultWriter &writer);  // Change return type from auto
  auto CreateDatabase(const std::string &db_name) -> bool;
  auto UseDatabase(const std::string &db_name) -> bool;
  auto GetCurrentDatabase() const -> std::string { return current_database_; }

  /**
   * Execute a SQL query in the HMSSQL instance.
   */
  auto ExecuteSql(const std::string &sql, ResultWriter &writer) -> bool;

  auto ExecuteSqlStatement(const std::string &sql, ResultWriter &writer) -> bool;

  /**
   * Execute a SQL query in the HMSSQL instance with provided txn.
   */
  auto ExecuteSqlTxn(const std::string &sql, ResultWriter &writer) -> bool;

  #ifndef ISDEBUG

  /**
   * FOR TEST ONLY. Generate test tables in this HMSSQL instance.
   * It's used in the shell to predefine some tables, as we don't support
   * create / drop table and insert for now. Should remove it in the future.
   */
  void GenerateTestTable();

  /**
   * FOR TEST ONLY. Generate mock tables in this HMSSQL instance.
   * It's used in the shell to predefine some tables, as we don't support
   * create / drop table and insert for now. Should remove it in the future.
   */
  void GenerateMockTable();

  #endif

  // TODO(chi): change to unique_ptr. Currently they're directly referenced by recovery test, so
  // we cannot do anything on them until someone decides to refactor the recovery test.

  DiskManager *disk_manager_;
  BufferPoolManager *buffer_pool_manager_;
  LogManager *log_manager_;
  CheckpointManager *checkpoint_manager_;
  Catalog *catalog_;
  ExecutionEngine *execution_engine_;
  std::shared_mutex catalog_lock_;

  auto GetSessionVariable(const std::string &key) -> std::string {
    if (session_variables_.find(key) != session_variables_.end()) {
      return session_variables_[key];
    }
    return "";
  }

  auto IsForceStarterRule() -> bool {
    auto variable = StringUtil::Lower(GetSessionVariable("force_optimizer_starter_rule"));
    return variable == "1" || variable == "true" || variable == "yes";
  }

private:
  void CmdDisplayTables(ResultWriter &writer);
  void CmdDisplayIndices(ResultWriter &writer);
  void CmdDisplayHelp(ResultWriter &writer);
  void WriteOneCell(const std::string &cell, ResultWriter &writer);
  std::unordered_map<std::string, std::string> session_variables_;
};

}  // namespace hmssql
