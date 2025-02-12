//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// binder/insert_statement.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../include/binder/bound_statement.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/catalog/column.h"
#include "../include/type/value.h"

namespace hmssql {

class SelectStatement;

class InsertStatement : public BoundStatement {
 public:
  explicit InsertStatement(std::unique_ptr<BoundBaseTableRef> table, std::unique_ptr<SelectStatement> select);

  std::unique_ptr<BoundBaseTableRef> table_;

  std::unique_ptr<SelectStatement> select_;

  auto ToString() const -> std::string override;
};

}  // namespace hmssql
