//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// table_heap.cpp
//
// Identification: src/storage/table/table_heap.cpp
//
//
//===----------------------------------------------------------------------===//

#include <cassert>

#include "fmt/format.h"
#include "../include/storage/table/table_heap.h"

namespace hmssql {

TableHeap::TableHeap(BufferPoolManager *buffer_pool_manager, LogManager *log_manager,
                    page_id_t first_page_id)
    : buffer_pool_manager_(buffer_pool_manager),
      log_manager_(log_manager),
      first_page_id_(first_page_id) {}

TableHeap::TableHeap(BufferPoolManager *buffer_pool_manager, LogManager *log_manager)
    : buffer_pool_manager_(buffer_pool_manager), log_manager_(log_manager) {
  // Initialize the first table page.
  auto first_page = reinterpret_cast<TablePage *>(buffer_pool_manager_->NewPage(&first_page_id_));
  BUSTUB_ASSERT(first_page != nullptr,
                "Couldn't create a page for the table heap. Have you completed the buffer pool manager project?");
  first_page->Init(first_page_id_, BUSTUB_PAGE_SIZE, INVALID_LSN, log_manager_);
  buffer_pool_manager_->UnpinPage(first_page_id_, true);
}

auto TableHeap::InsertTuple(const Tuple &tuple, RID *rid) -> bool {
  if (tuple.size_ + 32 > BUSTUB_PAGE_SIZE) {  // larger than one page size
    return false;
  }

  auto cur_page = static_cast<TablePage *>(buffer_pool_manager_->FetchPage(first_page_id_));
  if (cur_page == nullptr) {
    return false;
  }

  cur_page->WLatch();

  // Insert into the first page with enough space. If no such page exists, create a new page and insert into that.
  // INVARIANT: cur_page is WLatched if you leave the loop normally.
  while (!cur_page->InsertTuple(tuple, rid, log_manager_)) {
    auto next_page_id = cur_page->GetNextPageId();
    // If the next page is a valid page,
    if (next_page_id != INVALID_PAGE_ID) {
      auto next_page = static_cast<TablePage *>(buffer_pool_manager_->FetchPage(next_page_id));
      next_page->WLatch();
      // Unlatch and unpin the current page.
      cur_page->WUnlatch();
      buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), false);
      cur_page = next_page;
    } else {
      // Otherwise we have run out of valid pages. We need to create a new page.
      auto new_page = static_cast<TablePage *>(buffer_pool_manager_->NewPage(&next_page_id));
      // If we could not create a new page,
      if (new_page == nullptr) {
        // Then life sucks and we abort the transaction.
        cur_page->WUnlatch();
        buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), false);
        return false;
      }
      // Otherwise we were able to create a new page. We initialize it now.
      new_page->WLatch();
      cur_page->SetNextPageId(next_page_id);
      new_page->Init(next_page_id, BUSTUB_PAGE_SIZE, cur_page->GetTablePageId(), log_manager_);
      cur_page->WUnlatch();
      buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), true);
      cur_page = new_page;
    }
  }
  // This line has caused most of us to double-take and "whoa double unlatch".
  // We are not, in fact, double unlatching. See the invariant above.
  cur_page->WUnlatch();
  buffer_pool_manager_->UnpinPage(cur_page->GetTablePageId(), true);
  return true;
}

auto TableHeap::MarkDelete(const RID &rid) -> bool {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  if (page == nullptr) {
    return false;
  }
  
  // Mark the tuple as deleted
  page->WLatch();
  bool is_deleted = page->MarkDelete(rid);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
  return is_deleted;
}

auto TableHeap::UpdateTuple(const Tuple &tuple, const RID &rid) -> bool {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  // If the page could not be found, then abort the transaction.
  if (page == nullptr) {
    return false;
  }
  // Update the tuple; but first save the old value for rollbacks.
  Tuple old_tuple;
  page->WLatch();
  bool is_updated = page->UpdateTuple(tuple, &old_tuple, rid);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), is_updated);
  return is_updated;
}

void TableHeap::ApplyDelete(const RID &rid) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  BUSTUB_ASSERT(page != nullptr, "Couldn't find a page containing that RID.");
  // Delete the tuple from the page.
  page->WLatch();
  page->ApplyDelete(rid);
  /** Commented out to make compatible with p4; This is called only on commit or delete, which consequently unlocks the
   * tuple; so should be fine */
  // lock_manager_->Unlock(txn, rid);
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
}

void TableHeap::RollbackDelete(const RID &rid) {
  // Find the page which contains the tuple.
  auto page = reinterpret_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  BUSTUB_ASSERT(page != nullptr, "Couldn't find a page containing that RID.");
  // Rollback the delete.
  page->WLatch();
  page->WUnlatch();
  buffer_pool_manager_->UnpinPage(page->GetTablePageId(), true);
}

auto TableHeap::GetTuple(const RID &rid, Tuple *tuple, bool acquire_read_lock) -> bool {
  // Find the page which contains the tuple.
  auto page = static_cast<TablePage *>(buffer_pool_manager_->FetchPage(rid.GetPageId()));
  // If the page could not be found, then abort the transaction.
  if (page == nullptr) {
    return false;
  }
  // Read the tuple from the page.
  if (acquire_read_lock) {
    page->RLatch();
  }
  bool res = page->GetTuple(rid, tuple);
  if (acquire_read_lock) {
    page->RUnlatch();
  }
  buffer_pool_manager_->UnpinPage(rid.GetPageId(), false);
  return res;
}

auto TableHeap::Begin() -> TableIterator {
  // Start an iterator from the first page.
  // TODO(Wuwen): Hacky fix for now. Removing empty pages is a better way to handle this.
  RID rid;
  auto page_id = first_page_id_;
  while (page_id != INVALID_PAGE_ID) {
    auto page = static_cast<TablePage *>(buffer_pool_manager_->FetchPage(page_id));
    page->RLatch();
    // If this fails because there is no tuple, then RID will be the default-constructed value, which means EOF.
    auto found_tuple = page->GetFirstTupleRid(&rid);
    page->RUnlatch();
    buffer_pool_manager_->UnpinPage(page_id, false);
    if (found_tuple) {
      break;
    }
    page_id = page->GetNextPageId();
  }
  return {this, rid};
}

auto TableHeap::End() -> TableIterator { return {this, RID(INVALID_PAGE_ID, 0)}; }

}  // namespace hmssql
