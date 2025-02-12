//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// like_expression.h
//
// Identification: src/include/execution/expressions/like_expression.h
//
// Copyright (c) 2015-2021, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include "../include/execution/expressions/abstract_expression.h"
#include "../include/type/value.h"

namespace hmssql {

/**
 * LikeExpression represents a SQL LIKE expression.
 */
class LikeExpression : public AbstractExpression {
 public:
  /**
   * Constructs a new LikeExpression instance.
   * @param left The left-hand side of the LIKE expression
   * @param right The right-hand side of the LIKE expression
   */
  LikeExpression(AbstractExpressionRef left, AbstractExpressionRef right)
      : AbstractExpression({std::move(left), std::move(right)}, TypeId::BOOLEAN) {}

  auto Evaluate(const Tuple *tuple, const Schema &schema) const -> Value override {
    auto left_val = children_[0]->Evaluate(tuple, schema);
    auto right_val = children_[1]->Evaluate(tuple, schema);
    if (left_val.GetTypeId() != TypeId::VARCHAR) {
      left_val = Value(TypeId::VARCHAR, left_val.ToString());
    }
    if (right_val.GetTypeId() != TypeId::VARCHAR) {
      right_val = Value(TypeId::VARCHAR, right_val.ToString());
    }
    auto cmp_result = left_val.Like(right_val);
    return Value(TypeId::BOOLEAN, cmp_result == CmpBool::CmpTrue);
  }

  auto EvaluateJoin(const Tuple *left_tuple, const Schema &left_schema, const Tuple *right_tuple,
                    const Schema &right_schema) const -> Value override {
    auto left_val = children_[0]->EvaluateJoin(left_tuple, left_schema, right_tuple, right_schema);
    auto right_val = children_[1]->EvaluateJoin(left_tuple, left_schema, right_tuple, right_schema);
    if (left_val.GetTypeId() != TypeId::VARCHAR) {
      left_val = Value(TypeId::VARCHAR, left_val.ToString());
    }
    if (right_val.GetTypeId() != TypeId::VARCHAR) {
      right_val = Value(TypeId::VARCHAR, right_val.ToString());
    }
    auto cmp_result = left_val.Like(right_val);
    return Value(TypeId::BOOLEAN, cmp_result == CmpBool::CmpTrue);
  }

  auto CloneWithChildren(std::vector<AbstractExpressionRef> children) const
      -> std::unique_ptr<AbstractExpression> override {
    BUSTUB_ASSERT(children.size() == 2, "LikeExpression should have exactly two children.");
    return std::make_unique<LikeExpression>(std::move(children[0]), std::move(children[1]));
  }

  auto ToString() const -> std::string override {
    return fmt::format("LIKE({}, {})", children_[0]->ToString(), children_[1]->ToString());
  }
};

}  // namespace hmssql