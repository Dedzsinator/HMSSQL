//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// transaction_manager.cpp
//
// Identification: src/concurrency/transaction_manager.cpp
//
//
//===----------------------------------------------------------------------===//

#include "../include/concurrency/transaction_manager.h"

#include <mutex>  // NOLINT
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "../include/catalog/catalog.h"
#include "../include/storage/table/table_heap.h"
namespace hmssql {

std::unordered_map<txn_id_t, Transaction *> TransactionManager::txn_map = {};
std::shared_mutex TransactionManager::txn_map_mutex = {};

auto TransactionManager::Begin(Transaction *txn, IsolationLevel isolation_level) -> Transaction * {
  // Acquire the global transaction latch in shared mode.
  global_txn_latch_.RLock();

  if (txn == nullptr) {
      txn = new Transaction(next_txn_id_++, isolation_level);
  }

  if (enable_logging) {
      // Use the tag-based constructor
      LogRecord record(
          txn->GetTransactionId(),
          txn->GetPrevLSN(),
          LogRecordType::BEGIN,
          TRANSACTION_TAG  // Add the transaction tag
      );
      lsn_t lsn = log_manager_->AppendLogRecord(&record);
      txn->SetPrevLSN(lsn);
  }

  std::unique_lock<std::shared_mutex> l(txn_map_mutex);
  txn_map[txn->GetTransactionId()] = txn;
  return txn;
}

void TransactionManager::Commit(Transaction *txn) {
  txn->SetState(TransactionState::COMMITTED);

  // Perform all deletes before we commit.
  auto write_set = txn->GetWriteSet();
  while (!write_set->empty()) {
    auto &item = write_set->back();
    auto *table = item.table_;
    if (item.wtype_ == WType::DELETE) {
      // Note that this also releases the lock when holding the page latch.
      table->ApplyDelete(item.rid_, txn);
    }
    write_set->pop_back();
  }
  write_set->clear();

  // Release all the locks.
  ReleaseLocks(txn);
  // Release the global transaction latch.
  global_txn_latch_.RUnlock();
}

void TransactionManager::Abort(Transaction *txn) {
  txn->SetState(TransactionState::ABORTED);
  // Rollback before releasing the lock.
  auto table_write_set = txn->GetWriteSet();
  while (!table_write_set->empty()) {
    auto &item = table_write_set->back();
    auto *table = item.table_;
    if (item.wtype_ == WType::DELETE) {
      table->RollbackDelete(item.rid_, txn);
    } else if (item.wtype_ == WType::INSERT) {
      // Note that this also releases the lock when holding the page latch.
      table->ApplyDelete(item.rid_, txn);
    } else if (item.wtype_ == WType::UPDATE) {
      table->UpdateTuple(item.tuple_, item.rid_, txn);
    }
    table_write_set->pop_back();
  }
  table_write_set->clear();
  // Rollback index updates
  auto index_write_set = txn->GetIndexWriteSet();
  while (!index_write_set->empty()) {
    auto &item = index_write_set->back();
    auto *catalog = item.catalog_;
    // Metadata identifying the table that should be deleted from.
    TableInfo *table_info = catalog->GetTable(item.table_oid_);
    IndexInfo *index_info = catalog->GetIndex(item.index_oid_);
    auto new_key = item.tuple_.KeyFromTuple(table_info->schema_, *(index_info->index_->GetKeySchema()),
                                            index_info->index_->GetKeyAttrs());
    if (item.wtype_ == WType::DELETE) {
      index_info->index_->InsertEntry(new_key, item.rid_, txn);
    } else if (item.wtype_ == WType::INSERT) {
      index_info->index_->DeleteEntry(new_key, item.rid_, txn);
    } else if (item.wtype_ == WType::UPDATE) {
      // Delete the new key and insert the old key
      index_info->index_->DeleteEntry(new_key, item.rid_, txn);
      auto old_key = item.old_tuple_.KeyFromTuple(table_info->schema_, *(index_info->index_->GetKeySchema()),
                                                  index_info->index_->GetKeyAttrs());
      index_info->index_->InsertEntry(old_key, item.rid_, txn);
    }
    index_write_set->pop_back();
  }
  table_write_set->clear();
  index_write_set->clear();

  // Release all the locks.
  ReleaseLocks(txn);
  // Release the global transaction latch.
  global_txn_latch_.RUnlock();
}

void TransactionManager::BlockAllTransactions() { global_txn_latch_.WLock(); }

void TransactionManager::BlockNewTransactions() {
  std::unique_lock<std::mutex> lock(txn_mutex_);
  transactions_blocked_ = true;
}

void TransactionManager::ResumeTransactions() {
  std::unique_lock<std::mutex> lock(txn_mutex_);
  transactions_blocked_ = false;
  txn_cv_.notify_all();
}

void TransactionManager::WaitForActiveTransactions() {
  std::unique_lock<std::mutex> lock(txn_mutex_);
  txn_cv_.wait(lock, [this] { return active_txn_count_ == 0; });
}

}  // namespace hmssql
