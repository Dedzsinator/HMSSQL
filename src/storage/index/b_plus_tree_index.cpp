//===----------------------------------------------------------------------===//
//
// Identification: src/index/b_plus_tree_index.cpp
//
//===----------------------------------------------------------------------===//

#include "../include/storage/index/b_plus_tree_index.h"

namespace hmssql {
/*
 * Constructor
 */
INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_INDEX_TYPE::BPlusTreeIndex(std::unique_ptr<IndexMetadata> &&metadata, BufferPoolManager *buffer_pool_manager)
    : Index(std::move(metadata)),
      comparator_(GetMetadata()->GetKeySchema()),
      container_(GetMetadata()->GetName(), buffer_pool_manager, comparator_) {}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_INDEX_TYPE::InsertEntry(const Tuple &key, RID rid) {
  // construct insert index key
  KeyType index_key;
  index_key.SetFromKey(key);

  container_.Insert(index_key, rid);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_INDEX_TYPE::DeleteEntry(const Tuple &key, RID rid) {
  // construct delete index key
  KeyType index_key;
  index_key.SetFromKey(key);

  container_.Remove(index_key);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_INDEX_TYPE::ScanKey(const Tuple &key, std::vector<RID> *result) {
  // construct scan index key
  KeyType index_key;
  index_key.SetFromKey(key);

  container_.GetValue(index_key, result);
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_INDEX_TYPE::GetBeginIterator() -> INDEXITERATOR_TYPE { return container_.Begin(); }

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_INDEX_TYPE::GetBeginIterator(const KeyType &key) -> INDEXITERATOR_TYPE { return container_.Begin(key); }

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_INDEX_TYPE::GetEndIterator() -> INDEXITERATOR_TYPE { return container_.End(); }

template class BPlusTreeIndex<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTreeIndex<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTreeIndex<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTreeIndex<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTreeIndex<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace hmssql
