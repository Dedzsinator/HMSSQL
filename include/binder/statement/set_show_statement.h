//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// binder/set_get_statement.h
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <utility>
#include <vector>

#include "../include/binder/bound_statement.h"
#include "../include/common/enums/statement_type.h"
#include "fmt/format.h"

namespace hmssql {

class VariableSetStatement : public BoundStatement {
 public:
  explicit VariableSetStatement(std::string variable, std::string value)
      : BoundStatement(StatementType::VARIABLE_SET_STATEMENT),
        variable_(std::move(variable)),
        value_(std::move(value)) {}

  std::string variable_;
  std::string value_;

  auto ToString() const -> std::string override {
    return fmt::format("BoundVariableSet {{ variable={}, value={} }}", variable_, value_);
  }
};

class VariableShowStatement : public BoundStatement {
 public:
  explicit VariableShowStatement(std::string variable)
      : BoundStatement(StatementType::VARIABLE_SHOW_STATEMENT), variable_(std::move(variable)) {}

  std::string variable_;

  auto ToString() const -> std::string override {
    return fmt::format("BoundVariableShow {{ variable={} }}", variable_);
  }
};

}  // namespace hmssql
