//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// table_iterator.h
//
// Identification: src/include/storage/table/table_iterator.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cassert>

#include "../include/common/rid.h"
#include "../include/storage/table/tuple.h"

namespace hmssql {

class TableHeap;

/**
 * TableIterator enables the sequential scan of a TableHeap.
 */
class TableIterator {
  friend class Cursor;

 public:
  TableIterator(TableHeap *table_heap, RID rid);

  TableIterator(const TableIterator &other)
      : table_heap_(other.table_heap_), tuple_(new Tuple(*other.tuple_)) {}

  ~TableIterator() { delete tuple_; }

  inline auto operator==(const TableIterator &itr) const -> bool {
    return tuple_->rid_.Get() == itr.tuple_->rid_.Get();
  }

  inline auto operator!=(const TableIterator &itr) const -> bool { return !(*this == itr); }

  auto operator*() -> const Tuple &;

  auto operator->() -> Tuple *;

  auto operator++() -> TableIterator &;

  auto operator++(int) -> TableIterator;

  auto operator=(const TableIterator &other) -> TableIterator & {
    table_heap_ = other.table_heap_;
    *tuple_ = *other.tuple_;
    return *this;
  }

 private:
  TableHeap *table_heap_;
  Tuple *tuple_;
};

}  // namespace hmssql
