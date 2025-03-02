//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// extendible_hash_table_index.h
//
// Identification: src/include/storage/index/extendible_hash_table_index.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../include/container/disk/hash/disk_extendible_hash_table.h"
#include "../include/container/hash/hash_function.h"
#include "../include/storage/index/index.h"

namespace hmssql {

#define HASH_TABLE_INDEX_TYPE ExtendibleHashTableIndex<KeyType, ValueType, KeyComparator>

template <typename KeyType, typename ValueType, typename KeyComparator>
class ExtendibleHashTableIndex : public Index {
 public:
  ExtendibleHashTableIndex(std::unique_ptr<IndexMetadata> &&metadata, BufferPoolManager *buffer_pool_manager,
                           const HashFunction<KeyType> &hash_fn);

  ~ExtendibleHashTableIndex() override = default;

  void InsertEntry(const Tuple &key, RID rid) override;

  void DeleteEntry(const Tuple &key, RID rid) override;

  void ScanKey(const Tuple &key, std::vector<RID> *result) override;

 protected:
  // comparator for key
  KeyComparator comparator_;
  // container
  DiskExtendibleHashTable<KeyType, ValueType, KeyComparator> container_;
};

}  // namespace hmssql
