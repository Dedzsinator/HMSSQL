#include <string>

#include "../include/common/exception.h"
#include "../include/common/rid.h"
#include "../include/storage/index/b_plus_tree.h"
#include "../include/storage/page/header_page.h"
#include "../include/common/logger.h" // Add logger header

namespace hmssql {

// Create a dedicated logger for B+ tree operations
static auto bplus_tree_logger = hmssql::GetLogger("bplus_tree");

INDEX_TEMPLATE_ARGUMENTS
BPLUSTREE_TYPE::BPlusTree(std::string name, BufferPoolManager *buffer_pool_manager, const KeyComparator &comparator,
                          int leaf_max_size, int internal_max_size)
    : index_name_(std::move(name)),
      root_page_id_(INVALID_PAGE_ID),
      buffer_pool_manager_(buffer_pool_manager),
      comparator_(comparator),
      leaf_max_size_(leaf_max_size),
      internal_max_size_(internal_max_size) {
  LOG_INFO(bplus_tree_logger, "Created B+ tree index '{}' with leaf_max_size={}, internal_max_size={}", 
           index_name_, leaf_max_size_, internal_max_size_);
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::ExportTreeAfterOperation(const std::string& operation) const {
  // Create a timestamped filename for the JSON export
  auto now = std::chrono::system_clock::now();
  auto time_t_now = std::chrono::system_clock::to_time_t(now);
  std::tm tm_now{};
  localtime_r(&time_t_now, &tm_now);
  
  std::ostringstream filename;
  filename << "logs/b_plus_tree_" << index_name_ << "_" << operation << "_"
           << std::put_time(&tm_now, "%Y%m%d_%H%M%S") 
           << "_root" << root_page_id_ << ".json";
  
  LOG_INFO(bplus_tree_logger, "Exporting B+ tree '{}' state after {} operation to {}", 
           index_name_, operation, filename.str());
  
  try {
    // Create the logs directory if it doesn't exist
    std::filesystem::create_directories("logs");
    
    // Export the tree to JSON
    ExportToJSON(filename.str(), true);
  } catch (const std::exception& e) {
    LOG_ERROR(bplus_tree_logger, "Failed to export tree after {} operation: {}", operation, e.what());
  }
}

/**
 * Helper function to decide whether current b+tree is empty
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::IsEmpty() const -> bool { return root_page_id_ == INVALID_PAGE_ID; }
/*****************************************************************************
 * SEARCH
 *****************************************************************************/
/**
 * Return the only value that associated with input key
 * This method is used for point query
 * @return : true means key exists
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetValue(const KeyType &key, std::vector<ValueType> *result) -> bool {
  LOG_DEBUG(bplus_tree_logger, "GetValue operation started for key in index '{}'", index_name_);
  
  root_page_id_latch_.RLock();
  auto leaf_page = FindLeaf(key, Operation::SEARCH);
  auto *node = reinterpret_cast<LeafPage *>(leaf_page->GetData());

  ValueType v;
  auto existed = node->Lookup(key, &v, comparator_);

  leaf_page->RUnlatch();
  buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), false);

  if (!existed) {
    LOG_DEBUG(bplus_tree_logger, "Key not found in index '{}'", index_name_);
    return false;
  }

  result->push_back(v);
  LOG_DEBUG(bplus_tree_logger, "Key found in index '{}', returning value", index_name_);
  return true;
}

/*****************************************************************************
 * INSERTION
 *****************************************************************************/
/**
 * Insert constant key & value pair into b+ tree
 * if current tree is empty, start new tree, update root page id and insert
 * entry, otherwise insert into leaf page.
 * @return: since we only support unique key, if user try to insert duplicate
 * keys return false, otherwise return true.
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Insert(const KeyType &key, const ValueType &value) -> bool {
  LOG_DEBUG(bplus_tree_logger, "Insert operation started for index '{}'", index_name_);
  
  root_page_id_latch_.WLock();
  
  if (IsEmpty()) {
    LOG_INFO(bplus_tree_logger, "Tree is empty, starting new tree for index '{}'", index_name_);
    StartNewTree(key, value);
    root_page_id_latch_.WUnlock();
    LOG_SUCCESS(bplus_tree_logger, "Successfully inserted key into new tree for index '{}'", index_name_);
    
    // Export tree state after insertion into empty tree
    ExportTreeAfterOperation("insert_new_tree");
    return true;
  }

  bool result = InsertIntoLeaf(key, value);
  if (result) {
    LOG_SUCCESS(bplus_tree_logger, "Successfully inserted key into index '{}'", index_name_);
  } else {
    LOG_WARN(bplus_tree_logger, "Duplicate key detected, insert failed for index '{}'", index_name_);
  }
  
  // Export tree state after insertion
  if (result) {
    ExportTreeAfterOperation("insert");
  } else {
    ExportTreeAfterOperation("insert_failed_duplicate");
  }
  
  return result;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::StartNewTree(const KeyType &key, const ValueType &value) {
  LOG_DEBUG(bplus_tree_logger, "Starting new tree for index '{}'", index_name_);
  
  auto page = buffer_pool_manager_->NewPage(&root_page_id_);

  if (page == nullptr) {
    LOG_ERROR(bplus_tree_logger, "Out of memory: Cannot allocate new root page for index '{}'", index_name_);
    throw Exception(ExceptionType::OUT_OF_MEMORY, "Cannot allocate new page");
  }

  auto *leaf = reinterpret_cast<LeafPage *>(page->GetData());
  leaf->Init(root_page_id_, INVALID_PAGE_ID, leaf_max_size_);
  LOG_DEBUG(bplus_tree_logger, "Initialized root leaf page {} for index '{}'", root_page_id_, index_name_);

  leaf->Insert(key, value, comparator_);
  LOG_DEBUG(bplus_tree_logger, "Inserted key into root leaf page {} for index '{}'", root_page_id_, index_name_);

  buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
  LOG_INFO(bplus_tree_logger, "New tree started with root page {} for index '{}'", root_page_id_, index_name_);
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::InsertIntoLeaf(const KeyType &key, const ValueType &value) -> bool {
  LOG_DEBUG(bplus_tree_logger, "Inserting into leaf for index '{}'", index_name_);
  
  auto leaf_page = FindLeaf(key, Operation::INSERT);
  auto *node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  LOG_DEBUG(bplus_tree_logger, "Found leaf page {} for insertion in index '{}'", leaf_page->GetPageId(), index_name_);

  auto size = node->GetSize();
  auto new_size = node->Insert(key, value, comparator_);
  LOG_DEBUG(bplus_tree_logger, "After insertion attempt: original size={}, new size={}", size, new_size);

  // duplicate key
  if (new_size == size) {
    LOG_WARN(bplus_tree_logger, "Duplicate key detected in leaf page {} for index '{}'", 
             leaf_page->GetPageId(), index_name_);
    leaf_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), false);
    return false;
  }

  // leaf is not full
  if (new_size < leaf_max_size_) {
    LOG_DEBUG(bplus_tree_logger, "Leaf page {} has space after insertion for index '{}'", 
              leaf_page->GetPageId(), index_name_);
    leaf_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), true);
    return true;
  }

  // leaf is full, need to split
  LOG_INFO(bplus_tree_logger, "Leaf page {} is full, splitting for index '{}'", 
           leaf_page->GetPageId(), index_name_);
  
  auto sibling_leaf_node = Split(node);
  LOG_DEBUG(bplus_tree_logger, "Created sibling leaf page {} during split for index '{}'", 
            sibling_leaf_node->GetPageId(), index_name_);
  
  sibling_leaf_node->SetNextPageId(node->GetNextPageId());
  node->SetNextPageId(sibling_leaf_node->GetPageId());
  LOG_DEBUG(bplus_tree_logger, "Updated node links: page {}'s next is now page {}", 
            leaf_page->GetPageId(), sibling_leaf_node->GetPageId());

  auto risen_key = sibling_leaf_node->KeyAt(0);
  LOG_DEBUG(bplus_tree_logger, "Inserting risen key into parent for index '{}'", index_name_);
  InsertIntoParent(node, risen_key, sibling_leaf_node);

  leaf_page->WUnlatch();
  buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(sibling_leaf_node->GetPageId(), true);
  LOG_INFO(bplus_tree_logger, "Split complete, insertion successful for index '{}'", index_name_);
  return true;
}

INDEX_TEMPLATE_ARGUMENTS
template <typename N>
auto BPLUSTREE_TYPE::Split(N *node) -> N * {
  LOG_DEBUG(bplus_tree_logger, "Splitting {} node in index '{}'", 
            (node->IsLeafPage() ? "leaf" : "internal"), index_name_);
  
  page_id_t page_id;
  auto page = buffer_pool_manager_->NewPage(&page_id);

  if (page == nullptr) {
    LOG_ERROR(bplus_tree_logger, "Out of memory: Cannot allocate new page during split for index '{}'", index_name_);
    throw Exception(ExceptionType::OUT_OF_MEMORY, "Cannot allocate new page");
  }
  LOG_DEBUG(bplus_tree_logger, "Allocated new page {} for split", page_id);

  N *new_node = reinterpret_cast<N *>(page->GetData());
  new_node->SetPageType(node->GetPageType());

  if (node->IsLeafPage()) {
    LOG_DEBUG(bplus_tree_logger, "Splitting leaf page for index '{}'", index_name_);
    auto *leaf = reinterpret_cast<LeafPage *>(node);
    auto *new_leaf = reinterpret_cast<LeafPage *>(new_node);

    new_leaf->Init(page->GetPageId(), node->GetParentPageId(), leaf_max_size_);
    LOG_DEBUG(bplus_tree_logger, "Initialized new leaf page {} with parent {}", 
              page->GetPageId(), node->GetParentPageId());
    
    leaf->MoveHalfTo(new_leaf);
    LOG_DEBUG(bplus_tree_logger, "Moved half of the entries from page {} to new leaf page {}", 
              leaf->GetPageId(), new_leaf->GetPageId());
  } else {
    LOG_DEBUG(bplus_tree_logger, "Splitting internal page for index '{}'", index_name_);
    auto *internal = reinterpret_cast<InternalPage *>(node);
    auto *new_internal = reinterpret_cast<InternalPage *>(new_node);

    new_internal->Init(page->GetPageId(), node->GetParentPageId(), internal_max_size_);
    LOG_DEBUG(bplus_tree_logger, "Initialized new internal page {} with parent {}", 
              page->GetPageId(), node->GetParentPageId());
    
    internal->MoveHalfTo(new_internal, buffer_pool_manager_);
    LOG_DEBUG(bplus_tree_logger, "Moved half of the entries from page {} to new internal page {}", 
              internal->GetPageId(), new_internal->GetPageId());
  }

  LOG_INFO(bplus_tree_logger, "Split complete, new node page ID: {}", new_node->GetPageId());
  return new_node;
}

INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertIntoParent(BPlusTreePage *old_node, const KeyType &key, BPlusTreePage *new_node) {
  LOG_DEBUG(bplus_tree_logger, "Inserting into parent: key between pages {} and {}", 
            old_node->GetPageId(), new_node->GetPageId());

  if (old_node->IsRootPage()) {
    LOG_INFO(bplus_tree_logger, "Creating new root for index '{}' as old node {} is the root", 
             index_name_, old_node->GetPageId());
    
    auto page = buffer_pool_manager_->NewPage(&root_page_id_);

    if (page == nullptr) {
      LOG_ERROR(bplus_tree_logger, "Out of memory: Cannot allocate new root page for index '{}'", index_name_);
      throw Exception(ExceptionType::OUT_OF_MEMORY, "Cannot allocate new page");
    }
    LOG_DEBUG(bplus_tree_logger, "Allocated new root page {}", root_page_id_);

    auto *new_root = reinterpret_cast<InternalPage *>(page->GetData());
    new_root->Init(root_page_id_, INVALID_PAGE_ID, internal_max_size_);
    LOG_DEBUG(bplus_tree_logger, "Initialized new root page {}", root_page_id_);

    new_root->PopulateNewRoot(old_node->GetPageId(), key, new_node->GetPageId());
    LOG_DEBUG(bplus_tree_logger, "Populated new root page {} with children {} and {}", 
              root_page_id_, old_node->GetPageId(), new_node->GetPageId());

    old_node->SetParentPageId(new_root->GetPageId());
    new_node->SetParentPageId(new_root->GetPageId());
    LOG_DEBUG(bplus_tree_logger, "Updated parent pointers for pages {} and {} to new root {}", 
              old_node->GetPageId(), new_node->GetPageId(), new_root->GetPageId());

    buffer_pool_manager_->UnpinPage(page->GetPageId(), true);
    UpdateRootPageId(0);
    LOG_INFO(bplus_tree_logger, "New root created and updated for index '{}'", index_name_);
    
    return;
  }

  auto parent_page = buffer_pool_manager_->FetchPage(old_node->GetParentPageId());
  auto *parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());
  LOG_DEBUG(bplus_tree_logger, "Fetched parent page {} for insertion", parent_node->GetPageId());

  if (parent_node->GetSize() < internal_max_size_) {
    LOG_DEBUG(bplus_tree_logger, "Parent page {} has space, inserting key directly", parent_node->GetPageId());
    parent_node->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
    LOG_DEBUG(bplus_tree_logger, "Inserted key into parent page {} between children {} and {}", 
              parent_node->GetPageId(), old_node->GetPageId(), new_node->GetPageId());
    
    buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
    return;
  }

  LOG_INFO(bplus_tree_logger, "Parent page {} is full, needs splitting", parent_node->GetPageId());
  auto *mem = new char[INTERNAL_PAGE_HEADER_SIZE + sizeof(MappingType) * (parent_node->GetSize() + 1)];
  auto *copy_parent_node = reinterpret_cast<InternalPage *>(mem);
  std::memcpy(mem, parent_page->GetData(), INTERNAL_PAGE_HEADER_SIZE + sizeof(MappingType) * (parent_node->GetSize()));
  
  LOG_DEBUG(bplus_tree_logger, "Created temporary copy of parent page {} for insertion", parent_node->GetPageId());
  copy_parent_node->InsertNodeAfter(old_node->GetPageId(), key, new_node->GetPageId());
  
  auto parent_new_sibling_node = Split(copy_parent_node);
  KeyType new_key = parent_new_sibling_node->KeyAt(0);
  
  LOG_DEBUG(bplus_tree_logger, "Split temporary parent, copying back to original page {}", parent_node->GetPageId());
  std::memcpy(parent_page->GetData(), mem,
              INTERNAL_PAGE_HEADER_SIZE + sizeof(MappingType) * copy_parent_node->GetMinSize());
  
  LOG_DEBUG(bplus_tree_logger, "Recursively inserting into higher level parent for index '{}'", index_name_);
  InsertIntoParent(parent_node, new_key, parent_new_sibling_node);
  
  buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
  buffer_pool_manager_->UnpinPage(parent_new_sibling_node->GetPageId(), true);
  delete[] mem;
  
  LOG_INFO(bplus_tree_logger, "Parent split and insertion complete for index '{}'", index_name_);
}

/*****************************************************************************
 * REMOVE
 *****************************************************************************/
/**
 * Delete key & value pair associated with input key
 * If current tree is empty, return immdiately.
 * If not, User needs to first find the right leaf page as deletion target, then
 * delete entry from leaf page. Remember to deal with redistribute or merge if
 * necessary.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::Remove(const KeyType &key) {
  LOG_DEBUG(bplus_tree_logger, "Remove operation started for index '{}'", index_name_);
  
  root_page_id_latch_.WLock();
  
  if (IsEmpty()) {
    LOG_DEBUG(bplus_tree_logger, "Tree is empty, nothing to remove from index '{}'", index_name_);
    root_page_id_latch_.WUnlock();
    return;
  }

  auto leaf_page = FindLeaf(key, Operation::DELETE, false, false);
  auto *node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  LOG_DEBUG(bplus_tree_logger, "Found leaf page {} for key removal in index '{}'", 
            leaf_page->GetPageId(), index_name_);

  auto original_size = node->GetSize();
  auto new_size = node->RemoveAndDeleteRecord(key, comparator_);
  
  // Key not found
  if (original_size == new_size) {
    LOG_WARN(bplus_tree_logger, "Key not found in leaf page {} for removal in index '{}'", 
             leaf_page->GetPageId(), index_name_);
    leaf_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), false);
    
    // Export tree state after failed removal
    ExportTreeAfterOperation("remove_failed_not_found");
    return;
  }
  
  LOG_DEBUG(bplus_tree_logger, "Key removed from leaf page {}, checking for redistribution/coalesce", 
            leaf_page->GetPageId());

  auto node_should_delete = CoalesceOrRedistribute(node);
  leaf_page->WUnlatch();

  if (node_should_delete) {
    LOG_INFO(bplus_tree_logger, "Deleting page {} after coalesce for index '{}'", 
             node->GetPageId(), index_name_);
    buffer_pool_manager_->DeletePage(node->GetPageId());
  }

  buffer_pool_manager_->UnpinPage(leaf_page->GetPageId(), true);
  LOG_SUCCESS(bplus_tree_logger, "Successfully removed key from index '{}'", index_name_);
  
  // Export tree state after successful removal
  ExportTreeAfterOperation("remove");
}

INDEX_TEMPLATE_ARGUMENTS
template <typename N>
auto BPLUSTREE_TYPE::CoalesceOrRedistribute(N *node) -> bool {
  LOG_DEBUG(bplus_tree_logger, "CoalesceOrRedistribute called on {} node with page_id={} for index '{}'",
           (node->IsLeafPage() ? "leaf" : "internal"), node->GetPageId(), index_name_);
  
  if (node->IsRootPage()) {
    LOG_DEBUG(bplus_tree_logger, "Node is root page, adjusting root for index '{}'", index_name_);
    auto root_should_delete = AdjustRoot(node);
    
    if (root_should_delete) {
      LOG_INFO(bplus_tree_logger, "Root page {} should be deleted after adjustment for index '{}'", 
              node->GetPageId(), index_name_);
    } else {
      LOG_INFO(bplus_tree_logger, "Root page {} preserved after adjustment for index '{}'", 
              node->GetPageId(), index_name_);
    }
    
    return root_should_delete;
  }

  if (node->GetSize() >= node->GetMinSize()) {
    LOG_DEBUG(bplus_tree_logger, "Node page {} has enough entries (size={}, min={}), no action needed for index '{}'", 
             node->GetPageId(), node->GetSize(), node->GetMinSize(), index_name_);
    return false;
  }

  LOG_INFO(bplus_tree_logger, "Node page {} needs redistribution/coalescing (size={}, min={}) for index '{}'", 
          node->GetPageId(), node->GetSize(), node->GetMinSize(), index_name_);

  auto parent_page = buffer_pool_manager_->FetchPage(node->GetParentPageId());
  auto *parent_node = reinterpret_cast<InternalPage *>(parent_page->GetData());
  auto idx = parent_node->ValueIndex(node->GetPageId());
  
  LOG_DEBUG(bplus_tree_logger, "Found node at index {} in parent page {} for index '{}'", 
           idx, parent_node->GetPageId(), index_name_);

  // Try to borrow from left sibling if not the leftmost child
  if (idx > 0) {
    auto sibling_page = buffer_pool_manager_->FetchPage(parent_node->ValueAt(idx - 1));
    sibling_page->WLatch();
    N *sibling_node = reinterpret_cast<N *>(sibling_page->GetData());
    
    LOG_DEBUG(bplus_tree_logger, "Examining left sibling page {} (size={}, min={}) for index '{}'", 
             sibling_node->GetPageId(), sibling_node->GetSize(), sibling_node->GetMinSize(), index_name_);

    // If sibling has enough entries, redistribute
    if (sibling_node->GetSize() > sibling_node->GetMinSize()) {
      LOG_INFO(bplus_tree_logger, "Redistributing from left sibling page {} to page {} for index '{}'", 
              sibling_node->GetPageId(), node->GetPageId(), index_name_);
              
      Redistribute(sibling_node, node, parent_node, idx, true);
      
      LOG_SUCCESS(bplus_tree_logger, "Successfully redistributed from left sibling for index '{}'", index_name_);
      buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
      sibling_page->WUnlatch();
      buffer_pool_manager_->UnpinPage(sibling_page->GetPageId(), true);
      return false;
    }

    // Otherwise coalesce with left sibling
    LOG_INFO(bplus_tree_logger, "Coalescing node page {} into left sibling page {} for index '{}'", 
            node->GetPageId(), sibling_node->GetPageId(), index_name_);
            
    auto parent_node_should_delete = Coalesce(sibling_node, node, parent_node, idx);

    if (parent_node_should_delete) {
      LOG_INFO(bplus_tree_logger, "Parent page {} should be deleted after coalescing for index '{}'", 
              parent_node->GetPageId(), index_name_);
      buffer_pool_manager_->DeletePage(parent_node->GetPageId());
    } else {
      LOG_INFO(bplus_tree_logger, "Parent page {} preserved after coalescing for index '{}'", 
              parent_node->GetPageId(), index_name_);
    }
    
    buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
    sibling_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(sibling_page->GetPageId(), true);
    LOG_SUCCESS(bplus_tree_logger, "Successfully coalesced with left sibling for index '{}'", index_name_);
    return true;
  }

  // Try to borrow from right sibling if not the rightmost child
  if (idx != parent_node->GetSize() - 1) {
    auto sibling_page = buffer_pool_manager_->FetchPage(parent_node->ValueAt(idx + 1));
    sibling_page->WLatch();
    N *sibling_node = reinterpret_cast<N *>(sibling_page->GetData());
    
    LOG_DEBUG(bplus_tree_logger, "Examining right sibling page {} (size={}, min={}) for index '{}'", 
             sibling_node->GetPageId(), sibling_node->GetSize(), sibling_node->GetMinSize(), index_name_);

    // If sibling has enough entries, redistribute
    if (sibling_node->GetSize() > sibling_node->GetMinSize()) {
      LOG_INFO(bplus_tree_logger, "Redistributing from right sibling page {} to page {} for index '{}'", 
              sibling_node->GetPageId(), node->GetPageId(), index_name_);
              
      Redistribute(sibling_node, node, parent_node, idx, false);
      
      LOG_SUCCESS(bplus_tree_logger, "Successfully redistributed from right sibling for index '{}'", index_name_);
      buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
      sibling_page->WUnlatch();
      buffer_pool_manager_->UnpinPage(sibling_page->GetPageId(), true);
      return false;
    }
    
    // Otherwise coalesce with right sibling
    LOG_INFO(bplus_tree_logger, "Coalescing right sibling page {} into node page {} for index '{}'", 
            sibling_node->GetPageId(), node->GetPageId(), index_name_);
            
    auto sibling_idx = parent_node->ValueIndex(sibling_node->GetPageId());
    auto parent_node_should_delete = Coalesce(node, sibling_node, parent_node, sibling_idx);
    
    LOG_DEBUG(bplus_tree_logger, "Deleting right sibling page {} after coalescing for index '{}'", 
             sibling_node->GetPageId(), index_name_);
    buffer_pool_manager_->DeletePage(sibling_node->GetPageId());
    
    if (parent_node_should_delete) {
      LOG_INFO(bplus_tree_logger, "Parent page {} should be deleted after coalescing for index '{}'", 
              parent_node->GetPageId(), index_name_);
      buffer_pool_manager_->DeletePage(parent_node->GetPageId());
    } else {
      LOG_INFO(bplus_tree_logger, "Parent page {} preserved after coalescing for index '{}'", 
              parent_node->GetPageId(), index_name_);
    }
    
    buffer_pool_manager_->UnpinPage(parent_page->GetPageId(), true);
    sibling_page->WUnlatch();
    buffer_pool_manager_->UnpinPage(sibling_page->GetPageId(), true);
    LOG_SUCCESS(bplus_tree_logger, "Successfully coalesced with right sibling for index '{}'", index_name_);
    return false;
  }

  LOG_WARN(bplus_tree_logger, "Node page {} couldn't be redistributed or coalesced for index '{}'",
          node->GetPageId(), index_name_);
  return false;
}

INDEX_TEMPLATE_ARGUMENTS
template <typename N>
auto BPLUSTREE_TYPE::Coalesce(N *neighbor_node, N *node,
                              BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *parent, int index) -> bool {
  LOG_DEBUG(bplus_tree_logger, "Coalesce: Moving all entries from page {} to neighbor page {} with parent key at index {} for index '{}'", 
           node->GetPageId(), neighbor_node->GetPageId(), index, index_name_);
  
  auto middle_key = parent->KeyAt(index);
  LOG_DEBUG(bplus_tree_logger, "Using parent middle key: {} for coalescing in index '{}'", middle_key, index_name_);

  if (node->IsLeafPage()) {
    LOG_DEBUG(bplus_tree_logger, "Coalescing leaf nodes for index '{}'", index_name_);
    auto *leaf_node = reinterpret_cast<LeafPage *>(node);
    auto *prev_leaf_node = reinterpret_cast<LeafPage *>(neighbor_node);
    
    LOG_DEBUG(bplus_tree_logger, "Moving {} entries from leaf page {} to neighbor leaf page {} for index '{}'",
             leaf_node->GetSize(), leaf_node->GetPageId(), prev_leaf_node->GetPageId(), index_name_);
    leaf_node->MoveAllTo(prev_leaf_node);
    
    LOG_DEBUG(bplus_tree_logger, "After move: neighbor leaf page {} now has {} entries for index '{}'",
             prev_leaf_node->GetPageId(), prev_leaf_node->GetSize(), index_name_);
  } else {
    LOG_DEBUG(bplus_tree_logger, "Coalescing internal nodes for index '{}'", index_name_);
    auto *internal_node = reinterpret_cast<InternalPage *>(node);
    auto *prev_internal_node = reinterpret_cast<InternalPage *>(neighbor_node);
    
    LOG_DEBUG(bplus_tree_logger, "Moving {} entries from internal page {} to neighbor internal page {} for index '{}'",
             internal_node->GetSize(), internal_node->GetPageId(), prev_internal_node->GetPageId(), index_name_);
    internal_node->MoveAllTo(prev_internal_node, middle_key, buffer_pool_manager_);
    
    LOG_DEBUG(bplus_tree_logger, "After move: neighbor internal page {} now has {} entries for index '{}'",
             prev_internal_node->GetPageId(), prev_internal_node->GetSize(), index_name_);
  }

  LOG_DEBUG(bplus_tree_logger, "Removing entry at index {} from parent page {} for index '{}'",
           index, parent->GetPageId(), index_name_);
  parent->Remove(index);
  
  LOG_DEBUG(bplus_tree_logger, "Checking if parent page {} needs coalescing after removal for index '{}'",
           parent->GetPageId(), index_name_);
  return CoalesceOrRedistribute(parent);
}

INDEX_TEMPLATE_ARGUMENTS
template <typename N>
void BPLUSTREE_TYPE::Redistribute(N *neighbor_node, N *node,
                                  BPlusTreeInternalPage<KeyType, page_id_t, KeyComparator> *parent, int index,
                                  bool from_prev) {
  LOG_DEBUG(bplus_tree_logger, "Redistribute: {} node page {} {} neighbor page {} for index '{}'",
           (node->IsLeafPage() ? "leaf" : "internal"), 
           node->GetPageId(),
           (from_prev ? "from previous" : "from next"),
           neighbor_node->GetPageId(), 
           index_name_);

  if (node->IsLeafPage()) {
    auto *leaf_node = reinterpret_cast<LeafPage *>(node);
    auto *neighbor_leaf_node = reinterpret_cast<LeafPage *>(neighbor_node);

    if (!from_prev) {
      LOG_DEBUG(bplus_tree_logger, "Moving first entry from next leaf page {} to end of leaf page {} for index '{}'", 
               neighbor_leaf_node->GetPageId(), leaf_node->GetPageId(), index_name_);
               
      neighbor_leaf_node->MoveFirstToEndOf(leaf_node);
      
      LOG_DEBUG(bplus_tree_logger, "Updating parent key at index {} to new first key {} in leaf page {} for index '{}'",
               index + 1, neighbor_leaf_node->KeyAt(0), neighbor_leaf_node->GetPageId(), index_name_);
      parent->SetKeyAt(index + 1, neighbor_leaf_node->KeyAt(0));
    } else {
      LOG_DEBUG(bplus_tree_logger, "Moving last entry from previous leaf page {} to front of leaf page {} for index '{}'", 
               neighbor_leaf_node->GetPageId(), leaf_node->GetPageId(), index_name_);
               
      neighbor_leaf_node->MoveLastToFrontOf(leaf_node);
      
      LOG_DEBUG(bplus_tree_logger, "Updating parent key at index {} to new first key {} in leaf page {} for index '{}'",
               index, leaf_node->KeyAt(0), leaf_node->GetPageId(), index_name_);
      parent->SetKeyAt(index, leaf_node->KeyAt(0));
    }
    
    LOG_SUCCESS(bplus_tree_logger, "Leaf redistribution complete for index '{}'", index_name_);
  } else {
    auto *internal_node = reinterpret_cast<InternalPage *>(node);
    auto *neighbor_internal_node = reinterpret_cast<InternalPage *>(neighbor_node);

    if (!from_prev) {
      LOG_DEBUG(bplus_tree_logger, "Moving first entry from next internal page {} to end of internal page {} using parent key {} for index '{}'", 
               neighbor_internal_node->GetPageId(), internal_node->GetPageId(), parent->KeyAt(index + 1), index_name_);
               
      neighbor_internal_node->MoveFirstToEndOf(internal_node, parent->KeyAt(index + 1), buffer_pool_manager_);
      
      LOG_DEBUG(bplus_tree_logger, "Updating parent key at index {} to new first key {} in internal page {} for index '{}'",
               index + 1, neighbor_internal_node->KeyAt(0), neighbor_internal_node->GetPageId(), index_name_);
      parent->SetKeyAt(index + 1, neighbor_internal_node->KeyAt(0));
    } else {
      LOG_DEBUG(bplus_tree_logger, "Moving last entry from previous internal page {} to front of internal page {} using parent key {} for index '{}'", 
               neighbor_internal_node->GetPageId(), internal_node->GetPageId(), parent->KeyAt(index), index_name_);
               
      neighbor_internal_node->MoveLastToFrontOf(internal_node, parent->KeyAt(index), buffer_pool_manager_);
      
      LOG_DEBUG(bplus_tree_logger, "Updating parent key at index {} to new first key {} in internal page {} for index '{}'",
               index, internal_node->KeyAt(0), internal_node->GetPageId(), index_name_);
      parent->SetKeyAt(index, internal_node->KeyAt(0));
    }
    
    LOG_SUCCESS(bplus_tree_logger, "Internal node redistribution complete for index '{}'", index_name_);
  }
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::AdjustRoot(BPlusTreePage *old_root_node) -> bool {
  LOG_DEBUG(bplus_tree_logger, "Adjusting root page {} for index '{}'", old_root_node->GetPageId(), index_name_);

  // Case 1: Internal node with only one child - make child the new root
  if (!old_root_node->IsLeafPage() && old_root_node->GetSize() == 1) {
    LOG_INFO(bplus_tree_logger, "Root internal page {} has only one child, promoting child as new root for index '{}'", 
            old_root_node->GetPageId(), index_name_);
            
    auto *root_node = reinterpret_cast<InternalPage *>(old_root_node);
    auto only_child_page = buffer_pool_manager_->FetchPage(root_node->ValueAt(0));
    auto *only_child_node = reinterpret_cast<BPlusTreePage *>(only_child_page->GetData());
    
    LOG_DEBUG(bplus_tree_logger, "Promoting page {} as new root for index '{}'", 
             only_child_node->GetPageId(), index_name_);
    only_child_node->SetParentPageId(INVALID_PAGE_ID);

    root_page_id_ = only_child_node->GetPageId();
    LOG_INFO(bplus_tree_logger, "New root page ID is {} for index '{}'", root_page_id_, index_name_);

    UpdateRootPageId(0);
    LOG_DEBUG(bplus_tree_logger, "Updated header page with new root page ID for index '{}'", index_name_);

    buffer_pool_manager_->UnpinPage(only_child_page->GetPageId(), true);
    LOG_SUCCESS(bplus_tree_logger, "Root adjustment complete, old root page {} should be deleted for index '{}'", 
               old_root_node->GetPageId(), index_name_);
    return true;  // Should delete old root
  }

  // Case 2: Empty leaf root - make tree empty
  if (old_root_node->IsLeafPage() && old_root_node->GetSize() == 0) {
    LOG_INFO(bplus_tree_logger, "Root leaf page {} is empty, tree becomes empty for index '{}'", 
            old_root_node->GetPageId(), index_name_);
            
    root_page_id_ = INVALID_PAGE_ID;
    UpdateRootPageId(0);
    
    LOG_INFO(bplus_tree_logger, "Tree now empty, root page ID is INVALID for index '{}'", index_name_);
    return true;  // Should delete old root
  }
  
  LOG_INFO(bplus_tree_logger, "Root page {} doesn't need adjustment for index '{}'", 
          old_root_node->GetPageId(), index_name_);
  return false;  // Should keep old root
}
/*****************************************************************************
 * INDEX ITERATOR
 *****************************************************************************/
/**
 * Input parameter is void, find the leaftmost leaf page first, then construct
 * index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin() -> INDEXITERATOR_TYPE {
  if (root_page_id_ == INVALID_PAGE_ID) {
    return INDEXITERATOR_TYPE(nullptr, nullptr);
  }
  root_page_id_latch_.RLock();
  // Fix the parameter mismatch by using true/false for bool parameters
  auto leftmost_page = FindLeaf(KeyType(), Operation::SEARCH, true, false);
  return INDEXITERATOR_TYPE(buffer_pool_manager_, leftmost_page, 0);
}

/**
 * Input parameter is low key, find the leaf page that contains the input key
 * first, then construct index iterator
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::Begin(const KeyType &key) -> INDEXITERATOR_TYPE {
  if (root_page_id_ == INVALID_PAGE_ID) {
    return INDEXITERATOR_TYPE(nullptr, nullptr);
  }
  root_page_id_latch_.RLock();
  auto leaf_page = FindLeaf(key, Operation::SEARCH, false, false);
  auto *leaf_node = reinterpret_cast<LeafPage *>(leaf_page->GetData());
  auto idx = leaf_node->KeyIndex(key, comparator_);
  return INDEXITERATOR_TYPE(buffer_pool_manager_, leaf_page, idx);
}

/**
 * Input parameter is void, construct an index iterator representing the end
 * of the key/value pair in the leaf node
 * @return : index iterator
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::End() -> INDEXITERATOR_TYPE {
  if (root_page_id_ == INVALID_PAGE_ID) {
    return INDEXITERATOR_TYPE(nullptr, nullptr);
  }
  root_page_id_latch_.RLock();
  // Fix the parameter mismatch by using true/false for bool parameters
  auto rightmost_page = FindLeaf(KeyType(), Operation::SEARCH, false, true);
  auto *leaf_node = reinterpret_cast<LeafPage *>(rightmost_page->GetData());
  return INDEXITERATOR_TYPE(buffer_pool_manager_, rightmost_page, leaf_node->GetSize());
}

INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::FindLeaf(const KeyType &key, Operation operation, bool leftMost,
                              bool rightMost) -> Page * {
  // Remove transaction check
  assert(operation == Operation::SEARCH ? !(leftMost && rightMost) : true);

  assert(root_page_id_ != INVALID_PAGE_ID);
  auto page = buffer_pool_manager_->FetchPage(root_page_id_);
  auto *node = reinterpret_cast<BPlusTreePage *>(page->GetData());
  if (operation == Operation::SEARCH) {
    root_page_id_latch_.RUnlock();
    page->RLatch();
  } else {
    page->WLatch();
    // No need for transaction related code
  }

  while (!node->IsLeafPage()) {
    auto *i_node = reinterpret_cast<InternalPage *>(node);

    page_id_t child_node_page_id;
    if (leftMost) {
      child_node_page_id = i_node->ValueAt(0);
    } else if (rightMost) {
      child_node_page_id = i_node->ValueAt(i_node->GetSize() - 1);
    } else {
      child_node_page_id = i_node->Lookup(key, comparator_);
    }
    assert(child_node_page_id > 0);

    auto child_page = buffer_pool_manager_->FetchPage(child_node_page_id);
    auto child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());

    if (operation == Operation::SEARCH) {
      child_page->RLatch();
      page->RUnlatch();
      buffer_pool_manager_->UnpinPage(page->GetPageId(), false);
    } else {
      child_page->WLatch();
      // Store parent pages in a local vector instead of transaction
      // Since this is a simplified version without transactions
    }

    page = child_page;
    node = child_node;
  }

  return page;
}

/**
 * @return Page id of the root of this tree
 */
INDEX_TEMPLATE_ARGUMENTS
auto BPLUSTREE_TYPE::GetRootPageId() -> page_id_t {
  root_page_id_latch_.RLock();
  root_page_id_latch_.RUnlock();
  return root_page_id_;
}

/*****************************************************************************
 * UTILITIES AND DEBUG
 *****************************************************************************/
/**
 * Update/Insert root page id in header page(where page_id = 0, header_page is
 * defined under include/page/header_page.h)
 * Call this method everytime root page id is changed.
 * @parameter: insert_record      default value is false. When set to true,
 * insert a record <index_name, root_page_id> into header page instead of
 * updating it.
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::UpdateRootPageId(int insert_record) {
  auto *header_page = static_cast<HeaderPage *>(buffer_pool_manager_->FetchPage(HEADER_PAGE_ID));
  if (insert_record != 0) {
    // create a new record<index_name + root_page_id> in header_page
    header_page->InsertRecord(index_name_, root_page_id_);
  } else {
    // update root_page_id in header_page
    header_page->UpdateRecord(index_name_, root_page_id_);
  }
  buffer_pool_manager_->UnpinPage(HEADER_PAGE_ID, true);
}

/**
 * This method is used for test only
 * Read data from file and insert one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::InsertFromFile(const std::string &file_name) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;

    KeyType index_key;
    index_key.SetFromInteger(key);
    RID rid(key);
    Insert(index_key, rid);
  }
}

/**
 * This method is used for test only
 * Read data from file and remove one by one
 */
INDEX_TEMPLATE_ARGUMENTS
void BPLUSTREE_TYPE::RemoveFromFile(const std::string &file_name) {
  int64_t key;
  std::ifstream input(file_name);
  while (input) {
    input >> key;
    KeyType index_key;
    index_key.SetFromInteger(key);
    Remove(index_key);
  }
}

INDEX_TEMPLATE_ARGUMENTS
std::string BPLUSTREE_TYPE::ExportToJSON(const std::string &json_file, bool pretty) const {
  nlohmann::json tree_json;
  
  // Add basic tree information
  tree_json["index_name"] = index_name_;
  tree_json["root_page_id"] = root_page_id_;
  tree_json["is_empty"] = IsEmpty();
  
  if (!IsEmpty()) {
    auto root_page = buffer_pool_manager_->FetchPage(root_page_id_);
    if (root_page != nullptr) {
      auto *root_node = reinterpret_cast<BPlusTreePage *>(root_page->GetData());
      tree_json["tree"] = ExportNodeToJSON(root_node, buffer_pool_manager_);
      buffer_pool_manager_->UnpinPage(root_page_id_, false);
    }
  }
  
  std::string json_str;
  if (pretty) {
    json_str = tree_json.dump(2);  // Indent with 2 spaces
  } else {
    json_str = tree_json.dump();
  }
  
  // Save to file if specified
  if (!json_file.empty()) {
    std::ofstream out(json_file);
    if (out.is_open()) {
      out << json_str;
      out.close();
      LOG_SUCCESS(bplus_tree_logger, "Successfully exported B+ tree '{}' to JSON file: {}", 
                 index_name_, json_file);
    } else {
      LOG_ERROR(bplus_tree_logger, "Failed to open file for writing B+ tree JSON: {}", json_file);
    }
  }
  
  return json_str;
}

INDEX_TEMPLATE_ARGUMENTS
nlohmann::json BPLUSTREE_TYPE::ExportNodeToJSON(BPlusTreePage *page, BufferPoolManager *bpm) const {
  nlohmann::json node_json;
  
  // Common attributes for all node types
  node_json["page_id"] = page->GetPageId();
  node_json["parent_id"] = page->GetParentPageId();
  node_json["size"] = page->GetSize();
  node_json["max_size"] = page->GetMaxSize();
  node_json["min_size"] = page->GetMinSize();
  
  if (page->IsLeafPage()) {
    // Leaf node specific attributes
    auto *leaf = reinterpret_cast<LeafPage *>(page);
    node_json["type"] = "leaf";
    node_json["next_page_id"] = leaf->GetNextPageId();
    
    // Export keys and values as JSON arrays of strings
    nlohmann::json keys = nlohmann::json::array();
    nlohmann::json values = nlohmann::json::array();
    
    for (int i = 0; i < leaf->GetSize(); i++) {
      // For keys, convert to string
      std::stringstream key_ss;
      key_ss << leaf->KeyAt(i);
      keys.push_back(key_ss.str());
      
      // For RID values, convert to string
      // Use GetItem() to get the key-value pair, then access the value (second element)
      const auto& item = leaf->GetItem(i);
      const RID& rid = item.second;
      
      std::stringstream value_ss;
      value_ss << "page:" << rid.GetPageId() << ",slot:" << rid.GetSlotNum();
      values.push_back(value_ss.str());
    }
    
    node_json["keys"] = keys;
    node_json["values"] = values;
  } else {
    // Internal node specific attributes
    auto *internal = reinterpret_cast<InternalPage *>(page);
    node_json["type"] = "internal";
    
    // Export keys as JSON array of strings
    nlohmann::json keys = nlohmann::json::array();
    nlohmann::json page_ids = nlohmann::json::array();
    
    for (int i = 1; i < internal->GetSize(); i++) {
      std::stringstream key_ss;
      key_ss << internal->KeyAt(i);
      keys.push_back(key_ss.str());
    }
    
    // Export values (child pointers) as JSON array
    for (int i = 0; i < internal->GetSize(); i++) {
      page_ids.push_back(internal->ValueAt(i));
    }
    
    node_json["keys"] = keys;
    node_json["values"] = page_ids;
    
    // Recursively export children
    nlohmann::json children = nlohmann::json::array();
    for (int i = 0; i < internal->GetSize(); i++) {
      page_id_t child_page_id = internal->ValueAt(i);
      auto child_page = bpm->FetchPage(child_page_id);
      if (child_page != nullptr) {
        auto *child_node = reinterpret_cast<BPlusTreePage *>(child_page->GetData());
        children.push_back(ExportNodeToJSON(child_node, bpm));
        bpm->UnpinPage(child_page_id, false);
      }
    }
    node_json["children"] = children;
  }
  
  return node_json;
}

template class BPlusTree<GenericKey<4>, RID, GenericComparator<4>>;
template class BPlusTree<GenericKey<8>, RID, GenericComparator<8>>;
template class BPlusTree<GenericKey<16>, RID, GenericComparator<16>>;
template class BPlusTree<GenericKey<32>, RID, GenericComparator<32>>;
template class BPlusTree<GenericKey<64>, RID, GenericComparator<64>>;

}  // namespace hmssql
