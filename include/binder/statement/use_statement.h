#pragma once

#include "binder/bound_statement.h"
#include <string>

namespace hmssql {

class UseStatement : public BoundStatement {
 public:
  explicit UseStatement(std::string database_name)
      : BoundStatement(StatementType::USE_STATEMENT),
        database_name_(std::move(database_name)) {}

  std::string database_name_;

  auto ToString() const -> std::string override {
    return fmt::format("USE {}", database_name_);
  }
};

}  // namespace hmssql