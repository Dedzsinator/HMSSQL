#pragma once

#include <string>
#include <utility>
#include "../include/binder/bound_expression.h"
#include "../include/common/exception.h"

namespace hmssql {

/**
 * The star in SELECT list, e.g. `SELECT * FROM x`.
 */
class BoundStar : public BoundExpression {
 public:
  BoundStar() : BoundExpression(ExpressionType::STAR) {}

  auto HasAggregation() const -> bool override {
    throw hmssql::Exception("`HasAggregation` should not have been called on `BoundStar`.");
  }

  auto ToString() const -> std::string override { return "*"; }
};
}  // namespace hmssql
