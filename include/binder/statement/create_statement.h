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
struct PGCreateViewStmt;
struct PGCreateTempTableStmt;
}  // namespace duckdb_libpgquery

namespace hmssql {

class CreateStatement : public BoundStatement {
 public:
  explicit CreateStatement(std::string table, std::vector<Column> columns);

  std::string table_;
  std::vector<Column> columns_;

  auto ToString() const -> std::string override;
};

class CreateViewStatement : public BoundStatement {
 public:
  explicit CreateViewStatement(std::string view, std::string query);

  std::string view_;
  std::string query_;

  auto ToString() const -> std::string override;
};

class CreateTempTableStatement : public BoundStatement {
 public:
  explicit CreateTempTableStatement(std::string table, std::vector<Column> columns);

  std::string table_;
  std::vector<Column> columns_;

  auto ToString() const -> std::string override;
};

}  // namespace hmssql