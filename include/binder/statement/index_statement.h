//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// binder/index_statement.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../include/binder/bound_statement.h"
#include "../include/binder/expressions/bound_column_ref.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/catalog/column.h"

namespace hmssql {

class IndexStatement : public BoundStatement {
 public:
  explicit IndexStatement(std::string index_name, std::unique_ptr<BoundBaseTableRef> table,
                          std::vector<std::unique_ptr<BoundColumnRef>> cols);

  /** Name of the index */
  std::string index_name_;

  /** Create on which table */
  std::unique_ptr<BoundBaseTableRef> table_;

  /** Name of the columns */
  std::vector<std::unique_ptr<BoundColumnRef>> cols_;

  auto ToString() const -> std::string override;
};

}  // namespace hmssql
