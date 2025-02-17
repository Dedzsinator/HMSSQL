#include <optional>
#include <shared_mutex>
#include <string>
#include <tuple>

#include "../include/binder/binder.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_statement.h"
#include "../include/binder/statement/create_statement.h"
#include "../include/binder/statement/explain_statement.h"
#include "../include/binder/statement/index_statement.h"
#include "../include/binder/statement/select_statement.h"
#include "../include/binder/statement/set_show_statement.h"
#include "../include/buffer/buffer_pool_manager_instance.h"
#include "../include/catalog/schema.h"
#include "../include/catalog/table_generator.h"
#include "../include/common/hmssql_instance.h"
#include "../include/common/enums/statement_type.h"
#include "../include/common/exception.h"
#include "../include/common/util/string_util.h"
#include "../include/concurrency/lock_manager.h"
#include "../include/concurrency/transaction.h"
#include "../include/execution/execution_engine.h"
#include "../include/execution/executor_context.h"
#include "../include/execution/executors/mock_scan_executor.h"
#include "../include/execution/expressions/abstract_expression.h"
#include "../include/execution/plans/abstract_plan.h"
#include "fmt/core.h"
#include "fmt/format.h"
#include "../include/optimizer/optimizer.h"
#include "../include/planner/planner.h"
#include "../include/recovery/checkpoint_manager.h"
#include "../include/recovery/log_manager.h"
#include "../include/storage/disk/disk_manager.h"
#include "../include/storage/disk/disk_manager_memory.h"
#include "../include/type/value_factory.h"
#include "../../third_party/spdlog/spdlog.h"
#include "../include/binder/statement/use_statement.h"
#include "../include/binder/statement/create_db_statement.h"
#include "../include/recovery/log_record.h"
namespace hmssql {

auto HMSSQL::MakeExecutorContext(Transaction *txn) -> std::unique_ptr<ExecutorContext> {
  return std::make_unique<ExecutorContext>(txn, catalog_, buffer_pool_manager_, txn_manager_, lock_manager_);
}

auto HMSSQL::UseDatabase(const std::string &db_name) -> bool {
  std::shared_lock<std::shared_mutex> lock(databases_lock_);
  if (databases_.find(db_name) == databases_.end()) {
    return false;  // Database doesn't exist
  }
  
  current_database_ = db_name;
  catalog_ = databases_[db_name].get();
  spdlog::info("Switched to database: {}", db_name);
  return true;
}

HMSSQL::HMSSQL(const std::string &db_file_name) {
  enable_logging = false;

  // Storage related.
  disk_manager_ = new DiskManager(db_file_name);

  // Log related.
  log_manager_ = new LogManager(disk_manager_);

  // We need more frames for GenerateTestTable to work. Therefore, we use 128 instead of the default
  // buffer pool size specified in `config.h`.
  try {
    buffer_pool_manager_ = new BufferPoolManagerInstance(128, disk_manager_, LRUK_REPLACER_K, log_manager_);
  } catch (NotImplementedException &e) {
    std::cerr << "BufferPoolManager is not implemented, only mock tables are supported." << std::endl;
    buffer_pool_manager_ = nullptr;
  }

  // Transaction (txn) related.
  lock_manager_ = new LockManager();
  txn_manager_ = new TransactionManager(lock_manager_, log_manager_);

  // Checkpoint related.
  checkpoint_manager_ = new CheckpointManager(txn_manager_, log_manager_, buffer_pool_manager_);

  current_database_ = "";
  databases_["default"] = std::unique_ptr<Catalog>(
    new Catalog(buffer_pool_manager_, lock_manager_, log_manager_)
  );
  UseDatabase("default");     // Use the default database

  // Execution engine.
  execution_engine_ = new ExecutionEngine(buffer_pool_manager_, txn_manager_, catalog_);
}

HMSSQL::HMSSQL() {
  enable_logging = false;

  // Storage related.
  disk_manager_ = new DiskManagerUnlimitedMemory();

  // Log related.
  log_manager_ = new LogManager(disk_manager_);

  // We need more frames for GenerateTestTable to work. Therefore, we use 128 instead of the default
  // buffer pool size specified in `config.h`.
  try {
    buffer_pool_manager_ = new BufferPoolManagerInstance(128, disk_manager_, LRUK_REPLACER_K, log_manager_);
  } catch (NotImplementedException &e) {
    std::cerr << "BufferPoolManager is not implemented, only mock tables are supported." << std::endl;
    buffer_pool_manager_ = nullptr;
  }

  // Transaction (txn) related.
  lock_manager_ = new LockManager();
  txn_manager_ = new TransactionManager(lock_manager_, log_manager_);

  // Checkpoint related.
  checkpoint_manager_ = new CheckpointManager(txn_manager_, log_manager_, buffer_pool_manager_);

  // Catalog.
  catalog_ = new Catalog(buffer_pool_manager_, lock_manager_, log_manager_);

  // Execution engine.
  execution_engine_ = new ExecutionEngine(buffer_pool_manager_, txn_manager_, catalog_);
}

HMSSQL::HMSSQL(const std::string& db_file, bool enable_logging) {
  // Create disk manager first
  disk_manager_ = new DiskManager(db_file);

  // Initialize log manager if logging is enabled
  if (enable_logging) {
      log_manager_ = new LogManager(disk_manager_);
      log_manager_->RunFlushThread();
  } else {
      log_manager_ = nullptr;
  }

  // Create buffer pool manager
  buffer_pool_manager_ = new BufferPoolManagerInstance(
      BUFFER_POOL_SIZE,
      disk_manager_,
      LRUK_REPLACER_K,
      log_manager_
  );
}

void HMSSQL::CmdDisplayTables(ResultWriter &writer) {
  auto table_names = catalog_->GetTableNames();
  writer.BeginTable(false);
  writer.BeginHeader();
  writer.WriteHeaderCell("oid");
  writer.WriteHeaderCell("name");
  writer.WriteHeaderCell("cols");
  writer.EndHeader();
  for (const auto &name : table_names) {
    writer.BeginRow();
    const auto *table_info = catalog_->GetTable(name);
    writer.WriteCell(fmt::format("{}", table_info->oid_));
    writer.WriteCell(table_info->name_);
    writer.WriteCell(table_info->schema_.ToString());
    writer.EndRow();
  }
  writer.EndTable();
}

void HMSSQL::CmdDisplayIndices(ResultWriter &writer) {
  auto table_names = catalog_->GetTableNames();
  writer.BeginTable(false);
  writer.BeginHeader();
  writer.WriteHeaderCell("table_name");
  writer.WriteHeaderCell("index_oid");
  writer.WriteHeaderCell("index_name");
  writer.WriteHeaderCell("index_cols");
  writer.EndHeader();
  for (const auto &table_name : table_names) {
    for (const auto *index_info : catalog_->GetTableIndexes(table_name)) {
      writer.BeginRow();
      writer.WriteCell(table_name);
      writer.WriteCell(fmt::format("{}", index_info->index_oid_));
      writer.WriteCell(index_info->name_);
      writer.WriteCell(index_info->key_schema_.ToString());
      writer.EndRow();
    }
  }
  writer.EndTable();
}

void HMSSQL::CmdDisplayDatabases(ResultWriter &writer) {
  std::shared_lock<std::shared_mutex> lock(databases_lock_);
  
  writer.BeginTable(false);
  writer.BeginHeader();
  writer.WriteHeaderCell("Database");
  writer.WriteHeaderCell("Tables");
  writer.EndHeader();

  for (const auto& [db_name, catalog_ptr] : databases_) {
    writer.BeginRow();
    writer.WriteCell(db_name);
    
    // Get tables for this database
    std::string tables;
    auto table_names = catalog_ptr->GetTableNames();
    for (size_t i = 0; i < table_names.size(); ++i) {
      tables += table_names[i];
      if (i < table_names.size() - 1) {
        tables += "\n└── ";
      }
    }
    
    if (tables.empty()) {
      tables = "(empty)";
    }
    
    writer.WriteCell(tables);
    writer.EndRow();
  }
  writer.EndTable();
}

void HMSSQL::WriteOneCell(const std::string &cell, ResultWriter &writer) {
  writer.BeginTable(true);
  writer.BeginRow();
  writer.WriteCell(cell);
  writer.EndRow();
  writer.EndTable();
}

void HMSSQL::CmdDisplayHelp(ResultWriter &writer) {
  std::string help = R"(Welcome to the HMSSQL shell!

\dc: show all databases and their tables
\dt: show all tables
\di: show all indices
\save: save current database state
\checkpoint: perform manual checkpoint
\help: show this message again

HMSSQL shell currently only supports a small set of Postgres queries. We'll set
up a doc describing the current status later. It will silently ignore some parts
of the query, so it's normal that you'll get a wrong result when executing
unsupported SQL queries. This shell will be able to run `create table` only
after you have completed the buffer pool manager. It will be able to execute SQL
queries after you have implemented necessary query executors. Use `explain` to
see the execution plan of your query.
)";
  WriteOneCell(help, writer);
}

auto HMSSQL::ExecuteSql(const std::string &sql, ResultWriter &writer) -> bool {
  auto txn = txn_manager_->Begin();
  auto result = ExecuteSqlTxn(sql, writer, txn);
  txn_manager_->Commit(txn);
  delete txn;
  return result;
}

auto HMSSQL::ExecuteSqlStatement(const std::string &sql, ResultWriter &writer, Transaction *txn) -> bool {
  std::string lower_sql = StringUtil::Lower(sql);

  if (!sql.empty() && sql[0] == '\\') {
    // Internal meta-commands, like in `psql`.
    if (sql == "\\dt") {
      CmdDisplayTables(writer);
      return true;
    }
    if (sql == "\\di") {
      CmdDisplayIndices(writer);
      return true;
    }

    if (sql == "\\dc") {
      CmdDisplayDatabases(writer);
      return true;
    }

    if (sql == "\\save") {
      if (SaveState()) {
          WriteOneCell("Database state saved successfully", writer);
      } else {
          WriteOneCell("Failed to save database state", writer);
      }
      return true;
    }

    if (sql == "\\checkpoint") {
        try {
            checkpoint_manager_->BeginCheckpoint();
            WriteOneCell("Checkpoint started", writer);
            checkpoint_manager_->EndCheckpoint();
            WriteOneCell("Checkpoint completed", writer);
            return true;
        } catch (const Exception& e) {
            WriteOneCell(fmt::format("Checkpoint failed: {}", e.what()), writer);
            return false;
        }
    }

    if (sql == "\\help") {
      CmdDisplayHelp(writer);
      return true;
    }
    if (sql == "\\exit") {
      return false;  // Indicate that the shell should exit
    }
    throw Exception(fmt::format("unsupported internal command: {}", sql));
  }

  // Check if a database is selected
  if (current_database_.empty()) {
    throw Exception("No database selected. Use 'USE database_name' to select a database.");
  }

  bool is_successful = true;

  std::shared_lock<std::shared_mutex> l(catalog_lock_);
  hmssql::Binder binder(*catalog_);
  binder.ParseAndSave(sql);
  l.unlock();

  for (auto *stmt : binder.statement_nodes_) {
    auto statement = binder.BindStatement(stmt);
    switch (statement->type_) {
      case StatementType::CREATE_DATABASE_STATEMENT: {
        const auto &create_db_stmt = dynamic_cast<const CreateDatabaseStatement &>(*statement);
        if (CreateDatabase(create_db_stmt.database_name_)) {
          WriteOneCell(fmt::format("Database '{}' created successfully", create_db_stmt.database_name_), writer);
        } else {
          throw Exception(fmt::format("Database '{}' already exists", create_db_stmt.database_name_));
        }
        continue;
      }

      case StatementType::CREATE_STATEMENT: {
        const auto &create_stmt = dynamic_cast<const CreateStatement &>(*statement);

        std::unique_lock<std::shared_mutex> l(catalog_lock_);
        auto info = catalog_->CreateTable(txn, create_stmt.table_, Schema(create_stmt.columns_));
        l.unlock();

        if (info == nullptr) {
          throw hmssql::Exception("Failed to create table");
        }
        WriteOneCell(fmt::format("Table created with id = {}", info->oid_), writer);
        continue;
      }

      case StatementType::USE_STATEMENT: {
        const auto &use_stmt = dynamic_cast<const UseStatement &>(*statement);
        if (UseDatabase(use_stmt.database_name_)) {
          WriteOneCell(fmt::format("Switched to database '{}'", use_stmt.database_name_), writer);
        } else {
          throw Exception(fmt::format("Database '{}' does not exist", use_stmt.database_name_));
        }
        continue;
      }

      case StatementType::INDEX_STATEMENT: {
        const auto &index_stmt = dynamic_cast<const IndexStatement &>(*statement);

        std::vector<uint32_t> col_ids;
        for (const auto &col : index_stmt.cols_) {
          auto idx = index_stmt.table_->schema_.GetColIdx(col->col_name_.back());
          col_ids.push_back(idx);
          if (index_stmt.table_->schema_.GetColumn(idx).GetType() != TypeId::INTEGER) {
            throw NotImplementedException("only support creating index on integer column");
          }
        }
        if (col_ids.size() != 1) {
          throw NotImplementedException("only support creating index with exactly one column");
        }
        auto key_schema = Schema::CopySchema(&index_stmt.table_->schema_, col_ids);

        std::unique_lock<std::shared_mutex> l(catalog_lock_);
        auto info = catalog_->CreateIndex<IntegerKeyType, IntegerValueType, IntegerComparatorType>(
            txn, index_stmt.index_name_, index_stmt.table_->table_, index_stmt.table_->schema_, key_schema, col_ids,
            INTEGER_SIZE, IntegerHashFunctionType{});
        l.unlock();

        if (info == nullptr) {
          throw hmssql::Exception("Failed to create index");
        }
        WriteOneCell(fmt::format("Index created with id = {}", info->index_oid_), writer);
        continue;
      }

      case StatementType::VARIABLE_SHOW_STATEMENT: {
        const auto &show_stmt = dynamic_cast<const VariableShowStatement &>(*statement);
        auto content = GetSessionVariable(show_stmt.variable_);
        WriteOneCell(fmt::format("{}={}", show_stmt.variable_, content), writer);
        continue;
      }

      case StatementType::VARIABLE_SET_STATEMENT: {
        const auto &set_stmt = dynamic_cast<const VariableSetStatement &>(*statement);
        session_variables_[set_stmt.variable_] = set_stmt.value_;
        continue;
      }
      
      case StatementType::EXPLAIN_STATEMENT: {
        const auto &explain_stmt = dynamic_cast<const ExplainStatement &>(*statement);
        std::string output;

        // Print binder result.
        if ((explain_stmt.options_ & ExplainOptions::BINDER) != 0) {
          output += "=== BINDER ===";
          output += "\n";
          output += explain_stmt.statement_->ToString();
          output += "\n";
        }

        std::shared_lock<std::shared_mutex> l(catalog_lock_);

        hmssql::Planner planner(*catalog_);
        planner.PlanQuery(*explain_stmt.statement_);

        bool show_schema = (explain_stmt.options_ & ExplainOptions::SCHEMA) != 0;

        // Print planner result.
        if ((explain_stmt.options_ & ExplainOptions::PLANNER) != 0) {
          output += "=== PLANNER ===";
          output += "\n";
          output += planner.plan_->ToString(show_schema);
          output += "\n";
        }

        // Print optimizer result.
        hmssql::Optimizer optimizer(*catalog_, IsForceStarterRule());
        auto optimized_plan = optimizer.Optimize(planner.plan_);

        l.unlock();

        if ((explain_stmt.options_ & ExplainOptions::OPTIMIZER) != 0) {
          output += "=== OPTIMIZER ===";
          output += "\n";
          output += optimized_plan->ToString(show_schema);
          output += "\n";
        }

        WriteOneCell(output, writer);

        continue;
      }
      default:
        break;
    }

    std::shared_lock<std::shared_mutex> l(catalog_lock_);

    // Plan the query.
    hmssql::Planner planner(*catalog_);
    planner.PlanQuery(*statement);

    // Optimize the query.
    hmssql::Optimizer optimizer(*catalog_, IsForceStarterRule());
    auto optimized_plan = optimizer.Optimize(planner.plan_);

    l.unlock();

    // Execute the query.
    auto exec_ctx = MakeExecutorContext(txn);
    std::vector<Tuple> result_set{};
    is_successful &= execution_engine_->Execute(optimized_plan, &result_set, txn, exec_ctx.get());

    // Return the result set as a vector of string.
    auto schema = planner.plan_->OutputSchema();

    // Generate header for the result set.
    writer.BeginTable(false);
    writer.BeginHeader();
    for (const auto &column : schema.GetColumns()) {
      writer.WriteHeaderCell(column.GetName());
    }
    writer.EndHeader();

    // Transforming result set into strings.
    for (const auto &tuple : result_set) {
      writer.BeginRow();
      for (uint32_t i = 0; i < schema.GetColumnCount(); i++) {
        writer.WriteCell(tuple.GetValue(&schema, i).ToString());
      }
      writer.EndRow();
    }
    writer.EndTable();
  }

  return is_successful;
}

auto HMSSQL::ExecuteSqlTxn(const std::string &sql, ResultWriter &writer, Transaction *txn) -> bool {
  try {
      // Create log record for transaction begin
      if (log_manager_ != nullptr) {
          LogRecord begin_record(
              txn->GetTransactionId(),
              INVALID_LSN,
              LogRecordType::BEGIN,
              TRANSACTION_TAG  // Use tag to specify constructor
          );
          log_manager_->AppendLogRecord(&begin_record);
      }

      auto result = ExecuteSqlStatement(sql, writer, txn);

      if (log_manager_ != nullptr) {
          LogRecord commit_record(
              txn->GetTransactionId(),
              txn->GetPrevLSN(),
              LogRecordType::COMMIT,
              TRANSACTION_TAG  // Use tag to specify constructor
          );
          log_manager_->AppendLogRecord(&commit_record);
          log_manager_->FlushAllLogs();
      }

      return result;

  } catch (const Exception& e) {
      if (log_manager_ != nullptr) {
          LogRecord abort_record(
              txn->GetTransactionId(),
              txn->GetPrevLSN(),
              LogRecordType::ABORT,
              TRANSACTION_TAG  // Use tag to specify constructor
          );
          log_manager_->AppendLogRecord(&abort_record);
          log_manager_->FlushAllLogs();
      }

      spdlog::error("Transaction failed: {}", e.what());
      return false;
  }
}

auto HMSSQL::Checkpoint() -> bool {
  if (!log_manager_) {
      return false;
  }

  try {
      checkpoint_manager_->BeginCheckpoint();
      buffer_pool_manager_->FlushAllPages();
      
      LogRecord checkpoint_record(
          INVALID_TXN_ID,
          INVALID_LSN,
          LogRecordType::CHECKPOINT,
          CHECKPOINT_TAG  // Use tag to specify constructor
      );
      log_manager_->AppendLogRecord(&checkpoint_record);
      log_manager_->FlushAllLogs();
      
      checkpoint_manager_->EndCheckpoint();
      return true;
  } catch (const Exception& e) {
      spdlog::error("Checkpoint failed: {}", e.what());
      return false;
  }
}

/**
 * FOR TEST ONLY. Generate test tables in this HMSSQL instance.
 * It's used in the shell to predefine some tables, as we don't support
 * create / drop table and insert for now. Should remove it in the future.
 */
void HMSSQL::GenerateTestTable() {
  auto txn = txn_manager_->Begin();
  auto exec_ctx = MakeExecutorContext(txn);
  TableGenerator gen{exec_ctx.get()};

  std::shared_lock<std::shared_mutex> l(catalog_lock_);
  gen.GenerateTestTables();
  l.unlock();

  txn_manager_->Commit(txn);
  delete txn;
}

/**
 * FOR TEST ONLY. Generate test tables in this HMSSQL instance.
 * It's used in the shell to predefine some tables, as we don't support
 * create / drop table and insert for now. Should remove it in the future.
 */
void HMSSQL::GenerateMockTable() {
  // The actual content generated by mock scan executors are described in `mock_scan_executor.cpp`.
  auto txn = txn_manager_->Begin();

  std::shared_lock<std::shared_mutex> l(catalog_lock_);
  for (auto table_name = &mock_table_list[0]; *table_name != nullptr; table_name++) {
    catalog_->CreateTable(txn, *table_name, GetMockTableSchemaOf(*table_name), false);
  }
  l.unlock();

  txn_manager_->Commit(txn);
  delete txn;
}

HMSSQL::~HMSSQL() {
  SaveState();

  if (enable_logging) {
      log_manager_->StopFlushThread();
  }

  // First clear databases since they contain catalogs
  databases_.clear();  // Add this line
  
  // Then delete the execution engine since it might reference the catalog
  delete execution_engine_;
  
  // Delete catalog_ only if it wasn't in the databases_ map
  if (catalog_ && databases_.empty()) {
    delete catalog_;
  }
  
  // Delete remaining components in reverse order of creation
  delete checkpoint_manager_;
  delete log_manager_;
  delete buffer_pool_manager_;
  delete lock_manager_;
  delete txn_manager_;
  delete disk_manager_;
}

auto HMSSQL::SaveState() -> bool {
  checkpoint_manager_->BeginCheckpoint();

  try {
    std::unique_lock<std::shared_mutex> lock(databases_lock_);
        
    // Open state file for writing
    std::ofstream out(state_file_, std::ios::binary);
    if (!out) {
        spdlog::error("Failed to open state file for writing");
        checkpoint_manager_->EndCheckpoint();
        return false;
    }

      // Write number of databases
      size_t num_databases = databases_.size();
      out.write(reinterpret_cast<const char*>(&num_databases), sizeof(num_databases));

      // Write each database
      for (const auto& [db_name, catalog_ptr] : databases_) {
          // Write database name
          size_t name_length = db_name.length();
          out.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
          out.write(db_name.c_str(), name_length);

          // Get tables for this database
          auto table_names = catalog_ptr->GetTableNames();
          size_t num_tables = table_names.size();
          out.write(reinterpret_cast<const char*>(&num_tables), sizeof(num_tables));

          // Write each table
          for (const auto& table_name : table_names) {
              const auto* table_info = catalog_ptr->GetTable(table_name);
              if (table_info == nullptr) {
                  continue;  // Skip if table info not found
              }
              
              // Write table name
              name_length = table_name.length();
              out.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
              out.write(table_name.c_str(), name_length);

              // Write table OID
              out.write(reinterpret_cast<const char*>(&table_info->oid_), sizeof(table_info->oid_));
              
              // Write schema column count
              size_t col_count = table_info->schema_.GetColumnCount();
              out.write(reinterpret_cast<const char*>(&col_count), sizeof(col_count));

              // Write column information
              for (uint32_t i = 0; i < col_count; i++) {
                  const auto& col = table_info->schema_.GetColumn(i);
                  // Write column name
                  name_length = col.GetName().length();
                  out.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
                  out.write(col.GetName().c_str(), name_length);
                  
                  // Write column type
                  auto type_id = col.GetType();
                  out.write(reinterpret_cast<const char*>(&type_id), sizeof(type_id));
              }
          }
      }

      // Write current database
      size_t curr_db_len = current_database_.length();
      out.write(reinterpret_cast<const char*>(&curr_db_len), sizeof(curr_db_len));
      out.write(current_database_.c_str(), curr_db_len);

      spdlog::info("State saved successfully");
        
        // End checkpoint after successful save
        checkpoint_manager_->EndCheckpoint();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("Error saving state: {}", e.what());
        checkpoint_manager_->EndCheckpoint();
        return false;
    }
}

auto HMSSQL::CreateDatabase(const std::string& db_name) -> bool {
  std::unique_lock<std::shared_mutex> lock(catalog_lock_);
  
  if (databases_.find(db_name) != databases_.end()) {
      return false;
  }

  // Log the creation
  if (log_manager_) {
      LogRecord create_db_record(
          INVALID_TXN_ID,        // No transaction ID for system operations
          INVALID_LSN,           // No previous LSN
          LogRecordType::CREATE_DATABASE,
          db_name                // Pass database name to new constructor
      );
      log_manager_->AppendLogRecord(&create_db_record);
  }

  // Create database catalog
  databases_[db_name] = std::make_unique<Catalog>(
      buffer_pool_manager_,
      lock_manager_,
      log_manager_
  );
  
  return true;
}

}  // namespace hmssql
