#pragma once

#include <string>
#include <utility>

#include "../include/binder/bound_expression.h"
#include "../include/type/value.h"

namespace hmssql {

class BoundExpression;

/**
 * A bound constant, e.g., `1`.
 */
class BoundConstant : public BoundExpression {
 public:
  explicit BoundConstant(const Value &val) : BoundExpression(ExpressionType::CONSTANT), val_(val) {}

  auto ToString() const -> std::string override { return val_.ToString(); }

  auto HasAggregation() const -> bool override { return false; }

  /** The constant being bound. */
  Value val_;
};
}  // namespace hmssql
