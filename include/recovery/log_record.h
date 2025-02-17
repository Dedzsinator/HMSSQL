//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// log_record.h
//
// Identification: src/include/recovery/log_record.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>
#include <string>

#include "../include/common/config.h"
#include "../include/storage/table/tuple.h"

namespace hmssql {
/** The type of the log record. */
enum class LogRecordType {
  INVALID = 0,
  INSERT,
  MARKDELETE,
  APPLYDELETE,
  ROLLBACKDELETE,
  UPDATE,
  BEGIN,
  COMMIT,
  ABORT,
  /** Creating a new page in the table heap. */
  NEWPAGE,
  CREATE_DATABASE,
  CHECKPOINT
};

/**
 * For every write operation on the table page, you should write ahead a corresponding log record.
 *
 * For EACH log record, HEADER is like (5 fields in common, 20 bytes in total).
 *---------------------------------------------
 * | size | LSN | transID | prevLSN | LogType |
 *---------------------------------------------
 * For insert type log record
 *---------------------------------------------------------------
 * | HEADER | tuple_rid | tuple_size | tuple_data(char[] array) |
 *---------------------------------------------------------------
 * For delete type (including markdelete, rollbackdelete, applydelete)
 *----------------------------------------------------------------
 * | HEADER | tuple_rid | tuple_size | tuple_data(char[] array) |
 *---------------------------------------------------------------
 * For update type log record
 *-----------------------------------------------------------------------------------
 * | HEADER | tuple_rid | tuple_size | old_tuple_data | tuple_size | new_tuple_data |
 *-----------------------------------------------------------------------------------
 * For new page type log record
 *--------------------------
 * | HEADER | prev_page_id |
 *--------------------------
 */
struct TransactionTag {};
struct CheckpointTag {};
static constexpr TransactionTag TRANSACTION_TAG;
static constexpr CheckpointTag CHECKPOINT_TAG;


class LogRecord {
  friend class LogManager;
  friend class LogRecovery;

 public:
  LogRecord() = default;

  // constructor for Transaction type(BEGIN/COMMIT/ABORT)
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type, TransactionTag)
    : size_(HEADER_SIZE),
      lsn_(INVALID_LSN),
      txn_id_(txn_id),
      prev_lsn_(prev_lsn),
      log_record_type_(log_record_type) {
      assert(log_record_type == LogRecordType::BEGIN || 
            log_record_type == LogRecordType::COMMIT || 
            log_record_type == LogRecordType::ABORT);
  }

  // constructor for INSERT/DELETE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type, const RID &rid, const Tuple &tuple)
      : txn_id_(txn_id), prev_lsn_(prev_lsn), log_record_type_(log_record_type) {
    if (log_record_type == LogRecordType::INSERT) {
      insert_rid_ = rid;
      insert_tuple_ = tuple;
    } else {
      assert(log_record_type == LogRecordType::APPLYDELETE || log_record_type == LogRecordType::MARKDELETE ||
             log_record_type == LogRecordType::ROLLBACKDELETE);
      delete_rid_ = rid;
      delete_tuple_ = tuple;
    }
    // calculate log record size
    size_ = HEADER_SIZE + sizeof(RID) + sizeof(int32_t) + tuple.GetLength();
  }

  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type, const std::string& db_name)
        : size_(HEADER_SIZE + db_name.length() + sizeof(size_t)),
          lsn_(INVALID_LSN),
          txn_id_(txn_id),
          prev_lsn_(prev_lsn),
          log_record_type_(log_record_type),
          database_name_(db_name) {
        assert(log_record_type == LogRecordType::CREATE_DATABASE);
    }

  // constructor for checkpoint
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type, CheckpointTag)
        : size_(HEADER_SIZE),
          lsn_(INVALID_LSN),
          txn_id_(txn_id),
          prev_lsn_(prev_lsn),
          log_record_type_(log_record_type) {
        assert(log_record_type == LogRecordType::CHECKPOINT);
    }


  // constructor for UPDATE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type, const RID &update_rid,
            const Tuple &old_tuple, const Tuple &new_tuple)
      : txn_id_(txn_id),
        prev_lsn_(prev_lsn),
        log_record_type_(log_record_type),
        update_rid_(update_rid),
        old_tuple_(old_tuple),
        new_tuple_(new_tuple) {
    // calculate log record size
    size_ = HEADER_SIZE + sizeof(RID) + old_tuple.GetLength() + new_tuple.GetLength() + 2 * sizeof(int32_t);
  }

  // constructor for NEWPAGE type
  LogRecord(txn_id_t txn_id, lsn_t prev_lsn, LogRecordType log_record_type, page_id_t prev_page_id, page_id_t page_id)
      : size_(HEADER_SIZE),
        txn_id_(txn_id),
        prev_lsn_(prev_lsn),
        log_record_type_(log_record_type),
        prev_page_id_(prev_page_id),
        page_id_(page_id) {
    // calculate log record size, header size + sizeof(prev_page_id) + sizeof(page_id)
    size_ = HEADER_SIZE + sizeof(page_id_t) * 2;
  }

  ~LogRecord() = default;

  inline auto GetDeleteTuple() -> Tuple & { return delete_tuple_; }

  inline auto GetDeleteRID() -> RID & { return delete_rid_; }

  inline auto GetInsertTuple() -> Tuple & { return insert_tuple_; }

  inline auto GetInsertRID() -> RID & { return insert_rid_; }

  inline auto GetOriginalTuple() -> Tuple & { return old_tuple_; }

  inline auto GetUpdateTuple() -> Tuple & { return new_tuple_; }

  inline auto GetUpdateRID() -> RID & { return update_rid_; }

  inline auto GetNewPageRecord() -> page_id_t { return prev_page_id_; }

  inline auto GetSize() -> int32_t { return size_; }

  inline auto GetLSN() const -> lsn_t { return lsn_; }

  inline auto GetTxnId() -> txn_id_t { return txn_id_; }

  inline auto GetPrevLSN() -> lsn_t { return prev_lsn_; }

  inline auto GetLogRecordType() -> LogRecordType & { return log_record_type_; }

  // For debug purpose
  inline auto ToString() const -> std::string {
    std::ostringstream os;
    os << "Log["
       << "size:" << size_ << ", "
       << "LSN:" << lsn_ << ", "
       << "transID:" << txn_id_ << ", "
       << "prevLSN:" << prev_lsn_ << ", "
       << "LogType:" << static_cast<int>(log_record_type_) << "]";

    return os.str();
  }

 private:
  // Keep members in initialization order
  int32_t size_{0};
  lsn_t lsn_{INVALID_LSN};
  txn_id_t txn_id_{INVALID_TXN_ID};
  lsn_t prev_lsn_{INVALID_LSN};
  LogRecordType log_record_type_{LogRecordType::INVALID};
  std::string database_name_;  // Move after log_record_type_

  // Case-specific members
  RID delete_rid_;
  Tuple delete_tuple_;
  RID insert_rid_;
  Tuple insert_tuple_;
  RID update_rid_;
  Tuple old_tuple_;
  Tuple new_tuple_;
  page_id_t prev_page_id_{INVALID_PAGE_ID};
  page_id_t page_id_{INVALID_PAGE_ID};

  static const int HEADER_SIZE = 20;
};  // namespace hmssql

}  // namespace hmssql
