//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// abstract_pool.h
//
// Identification: src/include/type/abstract_pool.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <cstdlib>

namespace hmssql {

// Interface of a memory pool that can quickly allocate chunks of memory
class AbstractPool {
 public:
  // Virtual destructor
  virtual ~AbstractPool() = default;

  /**
   * @brief Allocate a contiguous block of memory of the given size
   * @param size The size (in bytes) of memory to allocate
   * @return A non-null pointer if allocation is successful. A null pointer if
   * allocation fails.
   *
   * TODO: Provide good error codes for failure cases
   */
  virtual auto Allocate(size_t size) -> void * = 0;

  /**
   * @brief Returns the provided chunk of memory back into the pool
   */
  virtual void Free(void *ptr) = 0;
};

}  // namespace hmssql
