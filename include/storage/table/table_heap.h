//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// table_heap.h
//
// Identification: src/include/storage/table/table_heap.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "../include/buffer/buffer_pool_manager.h"
#include "../include/recovery/log_manager.h"
#include "../include/storage/page/table_page.h"
#include "../include/storage/table/table_iterator.h"
#include "../include/storage/table/tuple.h"

namespace hmssql {

/**
 * TableHeap represents a physical table on disk.
 * This is just a doubly-linked list of pages.
 */
class TableHeap {
  friend class TableIterator;

 public:
  ~TableHeap() = default;

  /**
   * Create a table heap without a transaction. (open table)
   * @param buffer_pool_manager the buffer pool manager
   * @param log_manager the log manager
   * @param first_page_id the id of the first page
   */
  TableHeap(BufferPoolManager *buffer_pool_manager, LogManager *log_manager,
            page_id_t first_page_id);

  /**
   * Create a table heap with a transaction. (create table)
   * @param buffer_pool_manager the buffer pool manager
   * @param log_manager the log manager
   * @param txn the creating transaction
   */
  TableHeap(BufferPoolManager *buffer_pool_manager, LogManager *log_manager);

  /**
   * Insert a tuple into the table. If the tuple is too large (>= page_size), return false.
   * @param tuple tuple to insert
   * @param[out] rid the rid of the inserted tuple
   * @return true iff the insert is successful
   */
  auto InsertTuple(const Tuple &tuple, RID *rid) -> bool;

  /**
   * Mark the tuple as deleted. The actual delete will occur when ApplyDelete is called.
   * @param rid resource id of the tuple of delete
   * @return true iff the delete is successful (i.e the tuple exists)
   */
  auto MarkDelete(const RID &rid) -> bool;  // for delete

  /**
   * if the new tuple is too large to fit in the old page, return false (will delete and insert)
   * @param tuple new tuple
   * @param rid rid of the old tuple
   * @return true is update is successful.
   */
  auto UpdateTuple(const Tuple &tuple, const RID &rid) -> bool;

  /**
   * Called on Commit/Abort to actually delete a tuple or rollback an insert.
   * @param rid rid of the tuple to delete
   */
  void ApplyDelete(const RID &rid);

  /**
   * Called on abort to rollback a delete.
   * @param rid rid of the deleted tuple.
   */
  void RollbackDelete(const RID &rid);

  /**
   * Read a tuple from the table.
   * @param rid rid of the tuple to read
   * @param tuple output variable for the tuple
   * @return true if the read was successful (i.e. the tuple exists)
   */
  auto GetTuple(const RID &rid, Tuple *tuple, bool acquire_read_lock = true) -> bool;

  /** @return the begin iterator of this table */
  auto Begin() -> TableIterator;

  /** @return the end iterator of this table */
  auto End() -> TableIterator;

  /** @return the id of the first page of this table */
  inline auto GetFirstPageId() const -> page_id_t { return first_page_id_; }

 private:
  BufferPoolManager *buffer_pool_manager_;
  LogManager *log_manager_;
  page_id_t first_page_id_{};
};

}  // namespace hmssql
