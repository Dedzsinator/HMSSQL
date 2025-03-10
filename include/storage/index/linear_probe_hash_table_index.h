//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// linear_probe_hash_table_index.h
//
// Identification: src/include/storage/index/linear_probe_hash_table_index.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../include/container/disk/hash/linear_probe_hash_table.h"
#include "../include/container/hash/hash_function.h"
#include "../include/storage/index/index.h"

namespace hmssql {

#define HASH_TABLE_INDEX_TYPE LinearProbeHashTableIndex<KeyType, ValueType, KeyComparator>

template <typename KeyType, typename ValueType, typename KeyComparator>
class LinearProbeHashTableIndex : public Index {
 public:
  LinearProbeHashTableIndex(std::unique_ptr<IndexMetadata> &&metadata, BufferPoolManager *buffer_pool_manager,
                            size_t num_buckets, const HashFunction<KeyType> &hash_fn);

  ~LinearProbeHashTableIndex() override = default;

  void InsertEntry(const Tuple &key, RID rid) override;

  void DeleteEntry(const Tuple &key, RID rid) override;

  void ScanKey(const Tuple &key, std::vector<RID> *result) override;

 protected:
  // comparator for key
  KeyComparator comparator_;
  // container
  LinearProbeHashTable<KeyType, ValueType, KeyComparator> container_;
};

}  // namespace hmssql
