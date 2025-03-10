//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// table_page.h
//
// Identification: src/include/storage/page/table_page.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstring>

#include "../include/common/rid.h"
#include "../include/recovery/log_manager.h"
#include "../include/storage/page/page.h"
#include "../include/storage/table/tuple.h"

static constexpr uint64_t DELETE_MASK = (1U << (8 * sizeof(uint32_t) - 1));

namespace hmssql {

/**
 * Slotted page format:
 *  ---------------------------------------------------------
 *  | HEADER | ... FREE SPACE ... | ... INSERTED TUPLES ... |
 *  ---------------------------------------------------------
 *                                ^
 *                                free space pointer
 *
 *  Header format (size in bytes):
 *  ----------------------------------------------------------------------------
 *  | PageId (4)| LSN (4)| PrevPageId (4)| NextPageId (4)| FreeSpacePointer(4) |
 *  ----------------------------------------------------------------------------
 *  ----------------------------------------------------------------
 *  | TupleCount (4) | Tuple_1 offset (4) | Tuple_1 size (4) | ... |
 *  ----------------------------------------------------------------
 *
 */
class TablePage : public Page {
 public:
  /**
   * Initialize the TablePage header.
   * @param page_id the page ID of this table page
   * @param page_size the size of this table page
   * @param prev_page_id the previous table page ID
   * @param log_manager the log manager in use
   */
  void Init(page_id_t page_id, uint32_t page_size, page_id_t prev_page_id, LogManager *log_manager);

  /** @return the page ID of this table page */
  auto GetTablePageId() -> page_id_t { return *reinterpret_cast<page_id_t *>(GetData()); }

  /** @return the page ID of the previous table page */
  auto GetPrevPageId() -> page_id_t { return *reinterpret_cast<page_id_t *>(GetData() + OFFSET_PREV_PAGE_ID); }

  /** @return the page ID of the next table page */
  auto GetNextPageId() -> page_id_t { return *reinterpret_cast<page_id_t *>(GetData() + OFFSET_NEXT_PAGE_ID); }

  /** Set the page id of the previous page in the table. */
  void SetPrevPageId(page_id_t prev_page_id) {
    memcpy(GetData() + OFFSET_PREV_PAGE_ID, &prev_page_id, sizeof(page_id_t));
  }

  /** Set the page id of the next page in the table. */
  void SetNextPageId(page_id_t next_page_id) {
    memcpy(GetData() + OFFSET_NEXT_PAGE_ID, &next_page_id, sizeof(page_id_t));
  }

  /**
   * Insert a tuple into the table.
   * @param tuple tuple to insert
   * @param[out] rid rid of the inserted tuple
   * @param log_manager log manager for logging
   * @return true if the insert is successful (i.e. there is enough space)
   */
  auto InsertTuple(const Tuple &tuple, RID *rid, LogManager *log_manager) -> bool;

  /**
   * Mark a tuple as deleted. This does not actually delete the tuple.
   * @param rid rid of the tuple to mark as deleted
   * @return true if marking the tuple as deleted is successful (i.e the tuple exists)
   */
  auto MarkDelete(const RID &rid) -> bool;

  /**
   * Update a tuple.
   * @param new_tuple new value of the tuple
   * @param[out] old_tuple old value of the tuple
   * @param rid rid of the tuple
   * @return true if updating the tuple succeeded
   */
  auto UpdateTuple(const Tuple &new_tuple, Tuple *old_tuple, const RID &rid) -> bool;

  /** To be called on commit or abort. Actually perform the delete or rollback an insert. */
  void ApplyDelete(const RID &rid);

  /**
   * Read a tuple from a table.
   * @param rid rid of the tuple to read
   * @param[out] tuple the tuple that was read
   * @return true if the read is successful (i.e. the tuple exists)
   */
  auto GetTuple(const RID &rid, Tuple *tuple) -> bool;

  /**
   * @param[out] first_rid the RID of the first tuple in this page
   * @return true if the first tuple exists, false otherwise
   */
  auto GetFirstTupleRid(RID *first_rid) -> bool;

  /**
   * @param cur_rid the RID of the current tuple
   * @param[out] next_rid the RID of the tuple following the current tuple
   * @return true if the next tuple exists, false otherwise
   */
  auto GetNextTupleRid(const RID &cur_rid, RID *next_rid) -> bool;

 private:
  static_assert(sizeof(page_id_t) == 4);

  static constexpr size_t SIZE_TABLE_PAGE_HEADER = 24;
  static constexpr size_t SIZE_TUPLE = 8;
  static constexpr size_t OFFSET_PREV_PAGE_ID = 8;
  static constexpr size_t OFFSET_NEXT_PAGE_ID = 12;
  static constexpr size_t OFFSET_FREE_SPACE = 16;
  static constexpr size_t OFFSET_TUPLE_COUNT = 20;
  static constexpr size_t OFFSET_TUPLE_OFFSET = 24;  // Naming things is hard.
  static constexpr size_t OFFSET_TUPLE_SIZE = 28;

  /** @return pointer to the end of the current free space, see header comment */
  auto GetFreeSpacePointer() -> uint32_t { return *reinterpret_cast<uint32_t *>(GetData() + OFFSET_FREE_SPACE); }

  /** Sets the pointer, this should be the end of the current free space. */
  void SetFreeSpacePointer(uint32_t free_space_pointer) {
    memcpy(GetData() + OFFSET_FREE_SPACE, &free_space_pointer, sizeof(uint32_t));
  }

  /**
   * @note returned tuple count may be an overestimate because some slots may be empty
   * @return at least the number of tuples in this page
   */
  auto GetTupleCount() -> uint32_t { return *reinterpret_cast<uint32_t *>(GetData() + OFFSET_TUPLE_COUNT); }

  /** Set the number of tuples in this page. */
  void SetTupleCount(uint32_t tuple_count) { memcpy(GetData() + OFFSET_TUPLE_COUNT, &tuple_count, sizeof(uint32_t)); }

  auto GetFreeSpaceRemaining() -> uint32_t {
    return GetFreeSpacePointer() - SIZE_TABLE_PAGE_HEADER - SIZE_TUPLE * GetTupleCount();
  }

  /** @return tuple offset at slot slot_num */
  auto GetTupleOffsetAtSlot(uint32_t slot_num) -> uint32_t {
    return *reinterpret_cast<uint32_t *>(GetData() + OFFSET_TUPLE_OFFSET + SIZE_TUPLE * slot_num);
  }

  /** Set tuple offset at slot slot_num. */
  void SetTupleOffsetAtSlot(uint32_t slot_num, uint32_t offset) {
    memcpy(GetData() + OFFSET_TUPLE_OFFSET + SIZE_TUPLE * slot_num, &offset, sizeof(uint32_t));
  }

  /** @return tuple size at slot slot_num */
  auto GetTupleSize(uint32_t slot_num) -> uint32_t {
    return *reinterpret_cast<uint32_t *>(GetData() + OFFSET_TUPLE_SIZE + SIZE_TUPLE * slot_num);
  }

  /** Set tuple size at slot slot_num. */
  void SetTupleSize(uint32_t slot_num, uint32_t size) {
    memcpy(GetData() + OFFSET_TUPLE_SIZE + SIZE_TUPLE * slot_num, &size, sizeof(uint32_t));
  }

  /** @return true if the tuple is deleted or empty */
  static auto IsDeleted(uint32_t tuple_size) -> bool {
    return static_cast<bool>(tuple_size & DELETE_MASK) || tuple_size == 0;
  }

  /** @return tuple size with the deleted flag set */
  static auto SetDeletedFlag(uint32_t tuple_size) -> uint32_t {
    return static_cast<uint32_t>(tuple_size | DELETE_MASK);
  }

  /** @return tuple size with the deleted flag unset */
  static auto UnsetDeletedFlag(uint32_t tuple_size) -> uint32_t {
    return static_cast<uint32_t>(tuple_size & (~DELETE_MASK));
  }
};
}  // namespace hmssql