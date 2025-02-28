//===----------------------------------------------------------------------===//
//
// Identification: src/include/index/index_iterator.h
//
//===----------------------------------------------------------------------===//
/**
 * index_iterator.h
 * For range scan of b+ tree
 */
#pragma once
#include "../include/storage/page/b_plus_tree_leaf_page.h"

namespace hmssql {

#define INDEXITERATOR_TYPE IndexIterator<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class IndexIterator {
 public:
  using LeafPage = BPlusTreeLeafPage<KeyType, ValueType, KeyComparator>;

  // you may define your own constructor based on your member variables
  IndexIterator(BufferPoolManager *bpm, Page *page, int index = 0);
  ~IndexIterator();

  auto IsEnd() -> bool;

  auto operator*() -> const MappingType &;

  auto operator++() -> IndexIterator &;

  auto operator==(const IndexIterator &itr) const -> bool;

  auto operator!=(const IndexIterator &itr) const -> bool;

 private:
  // add your own private member variables here
  BufferPoolManager *buffer_pool_manager_;
  Page *page_;
  LeafPage *leaf_ = nullptr;
  int index_ = 0;
};

}  // namespace hmssql
