//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// type_id.h
//
// Identification: src/include/type/type_id.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

namespace hmssql {
// Every possible SQL type ID
enum TypeId { INVALID = 0, BOOLEAN, TINYINT, SMALLINT, INTEGER, BIGINT, DECIMAL, VARCHAR, TIMESTAMP };
}  // namespace hmssql
