//===----------------------------------------------------------------------===//
//
// Identification: src/include/index/b_plus_tree_index.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../include/container/hash/hash_function.h"
#include "../include/storage/index/b_plus_tree.h"
#include "../include/storage/index/index.h"

namespace hmssql {

#define BPLUSTREE_INDEX_TYPE BPlusTreeIndex<KeyType, ValueType, KeyComparator>

INDEX_TEMPLATE_ARGUMENTS
class BPlusTreeIndex : public Index {
 public:
  BPlusTreeIndex(std::unique_ptr<IndexMetadata> &&metadata, BufferPoolManager *buffer_pool_manager);

  void InsertEntry(const Tuple &key, RID rid) override;

  void DeleteEntry(const Tuple &key, RID rid) override;

  void ScanKey(const Tuple &key, std::vector<RID> *result) override;

  auto GetBeginIterator() -> INDEXITERATOR_TYPE;

  auto GetBeginIterator(const KeyType &key) -> INDEXITERATOR_TYPE;

  auto GetEndIterator() -> INDEXITERATOR_TYPE;

 protected:
  // comparator for key
  KeyComparator comparator_;
  // container
  BPlusTree<KeyType, ValueType, KeyComparator> container_;
};

/** We only support index table with one integer key for now in HMSSQL. Hardcode everything here. */

constexpr static const auto INTEGER_SIZE = 4;
using IntegerKeyType = GenericKey<INTEGER_SIZE>;
using IntegerValueType = RID;
using IntegerComparatorType = GenericComparator<INTEGER_SIZE>;
using BPlusTreeIndexForOneIntegerColumn = BPlusTreeIndex<IntegerKeyType, IntegerValueType, IntegerComparatorType>;
using BPlusTreeIndexIteratorForOneIntegerColumn =
    IndexIterator<IntegerKeyType, IntegerValueType, IntegerComparatorType>;
using IntegerHashFunctionType = HashFunction<IntegerKeyType>;

}  // namespace hmssql
