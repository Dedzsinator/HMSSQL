#include <vector>

#include "../include/storage/index/extendible_hash_table_index.h"

namespace hmssql {
/*
 * Constructor
 */
template <typename KeyType, typename ValueType, typename KeyComparator>
HASH_TABLE_INDEX_TYPE::ExtendibleHashTableIndex(std::unique_ptr<IndexMetadata> &&metadata,
                                                BufferPoolManager *buffer_pool_manager,
                                                const HashFunction<KeyType> &hash_fn)
    : Index(std::move(metadata)),
      comparator_(GetMetadata()->GetKeySchema()),
      container_(GetMetadata()->GetName(), buffer_pool_manager, comparator_, hash_fn) {}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_INDEX_TYPE::InsertEntry(const Tuple &key, RID rid) {
  // construct insert index key
  KeyType index_key;
  index_key.SetFromKey(key);

  container_.Insert(index_key, rid);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_INDEX_TYPE::DeleteEntry(const Tuple &key, RID rid) {
  // construct delete index key
  KeyType index_key;
  index_key.SetFromKey(key);

  container_.Remove(index_key, rid);
}

template <typename KeyType, typename ValueType, typename KeyComparator>
void HASH_TABLE_INDEX_TYPE::ScanKey(const Tuple &key, std::vector<RID> *result) {
  // construct scan index key
  KeyType index_key;
  index_key.SetFromKey(key);

  container_.GetValue(index_key, result);
}
template class ExtendibleHashTableIndex<GenericKey<4>, RID, GenericComparator<4>>;
template class ExtendibleHashTableIndex<GenericKey<8>, RID, GenericComparator<8>>;
template class ExtendibleHashTableIndex<GenericKey<16>, RID, GenericComparator<16>>;
template class ExtendibleHashTableIndex<GenericKey<32>, RID, GenericComparator<32>>;
template class ExtendibleHashTableIndex<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace hmssql
