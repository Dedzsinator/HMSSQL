//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// binder/create_statement.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <vector>

#include "../include/binder/bound_statement.h"
#include "../include/catalog/column.h"

namespace duckdb_libpgquery {
struct PGCreateStmt;
}  // namespace duckdb_libpgquery

namespace hmssql {

class CreateStatement : public BoundStatement {
 public:
  explicit CreateStatement(std::string table, std::vector<Column> columns);

  std::string table_;
  std::vector<Column> columns_;

  auto ToString() const -> std::string override;
};

}  // namespace hmssql
