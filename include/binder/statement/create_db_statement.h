#pragma once

#include "binder/bound_statement.h"
#include <string>
#include "fmt/format.h"

namespace hmssql {

class CreateDatabaseStatement : public BoundStatement {
 public:
  explicit CreateDatabaseStatement(std::string database_name) 
    : BoundStatement(StatementType::CREATE_DATABASE_STATEMENT),
      database_name_(std::move(database_name)) {}

  std::string database_name_;

  auto ToString() const -> std::string override {
    return fmt::format("CreateDatabase {{ database={} }}", database_name_);
  }
};

} // namespace hmssql