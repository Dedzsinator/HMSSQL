//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// table_iterator.cpp
//
// Identification: src/storage/table/table_iterator.cpp
//
//
//===----------------------------------------------------------------------===//

#include <cassert>

#include "../include/common/exception.h"
#include "../include/storage/table/table_heap.h"

namespace hmssql {

TableIterator::TableIterator(TableHeap *table_heap, RID rid)
    : table_heap_(table_heap), tuple_(new Tuple(rid)) {
  if (rid.GetPageId() != INVALID_PAGE_ID) {
    if (!table_heap_->GetTuple(tuple_->rid_, tuple_)) {
      throw hmssql::Exception("read non-existing tuple");
    }
  }
}

auto TableIterator::operator*() -> const Tuple & {
  assert(*this != table_heap_->End());
  return *tuple_;
}

auto TableIterator::operator->() -> Tuple * {
  assert(*this != table_heap_->End());
  return tuple_;
}

auto TableIterator::operator++() -> TableIterator & {
  BufferPoolManager *buffer_pool_manager = table_heap_->buffer_pool_manager_;
  auto cur_page = static_cast<TablePage *>(buffer_pool_manager->FetchPage(tuple_->rid_.GetPageId()));
  BUSTUB_ENSURE(cur_page != nullptr, "BPM full");  // all pages are pinned

  cur_page->RLatch();
  RID next_tuple_rid;
  if (!cur_page->GetNextTupleRid(tuple_->rid_,
                                 &next_tuple_rid)) {  // end of this page
    while (cur_page->GetNextPageId() != INVALID_PAGE_ID) {
      auto next_page = static_cast<TablePage *>(buffer_pool_manager->FetchPage(cur_page->GetNextPageId()));
      cur_page->RUnlatch();
      buffer_pool_manager->UnpinPage(cur_page->GetTablePageId(), false);
      cur_page = next_page;
      cur_page->RLatch();
      if (cur_page->GetFirstTupleRid(&next_tuple_rid)) {
        break;
      }
    }
  }
  tuple_->rid_ = next_tuple_rid;

  if (*this != table_heap_->End()) {
    // DO NOT ACQUIRE READ LOCK twice in a single thread otherwise it may deadlock.
    // See https://users.rust-lang.org/t/how-bad-is-the-potential-deadlock-mentioned-in-rwlocks-document/67234
    if (!table_heap_->GetTuple(tuple_->rid_, tuple_, false)) {
      cur_page->RUnlatch();
      buffer_pool_manager->UnpinPage(cur_page->GetTablePageId(), false);
      throw hmssql::Exception("read non-existing tuple");
    }
  }
  // release until copy the tuple
  cur_page->RUnlatch();
  buffer_pool_manager->UnpinPage(cur_page->GetTablePageId(), false);
  return *this;
}

auto TableIterator::operator++(int) -> TableIterator {
  TableIterator clone(*this);
  ++(*this);
  return clone;
}

}  // namespace hmssql
