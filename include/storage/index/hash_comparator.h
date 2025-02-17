//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// hash_comparator.h
//
// Identification: src/include/storage/index/hash_comparator.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

namespace hmssql {

/**
 * HashComparator is for comparing hash_t, it returns:
 *  < 0 if lhs < rhs
 *  > 0 if lhs > rhs
 *  = 0 if lhs = rhs
 */
class HashComparator {
 public:
  inline auto operator()(const hash_t lhs, const hash_t rhs) -> int { return lhs < rhs ? -1 : (lhs > rhs ? 1 : 0); }
};

}  // namespace hmssql
