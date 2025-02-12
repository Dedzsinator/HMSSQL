#pragma once

#include <memory>
#include <string>
#include "../include/common/macros.h"
#include "fmt/format.h"

namespace hmssql {

/**
 * All types of expressions in binder.
 */
enum class ExpressionType : uint8_t {
  INVALID = 0,    /**< Invalid expression type. */
  CONSTANT = 1,   /**< Constant expression type. */
  COLUMN_REF = 3, /**< A column in a table. */
  TYPE_CAST = 4,  /**< Type cast expression type. */
  FUNCTION = 5,   /**< Function expression type. */
  AGG_CALL = 6,   /**< Aggregation function expression type. */
  STAR = 7,       /**< Star expression type, will be rewritten by binder and won't appear in plan. */
  UNARY_OP = 8,   /**< Unary expression type. */
  BINARY_OP = 9,  /**< Binary expression type. */
  ALIAS = 10,     /**< Alias expression type. */
};

/**
 * A bound expression.
 */
class BoundExpression {
 public:
  explicit BoundExpression(ExpressionType type) : type_(type) {}
  BoundExpression() = default;
  virtual ~BoundExpression() = default;

  virtual auto ToString() const -> std::string { return ""; };

  auto IsInvalid() const -> bool { return type_ == ExpressionType::INVALID; }

  virtual auto HasAggregation() const -> bool { UNREACHABLE("has aggregation should have been implemented!"); }

  /** The type of this expression. */
  ExpressionType type_{ExpressionType::INVALID};
};

}  // namespace hmssql

template <typename T>
struct fmt::formatter<T, std::enable_if_t<std::is_base_of<hmssql::BoundExpression, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const T &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x.ToString(), ctx);
  }
};

template <typename T>
struct fmt::formatter<std::unique_ptr<T>, std::enable_if_t<std::is_base_of<hmssql::BoundExpression, T>::value, char>>
    : fmt::formatter<std::string> {
  template <typename FormatCtx>
  auto format(const std::unique_ptr<T> &x, FormatCtx &ctx) const {
    return fmt::formatter<std::string>::format(x->ToString(), ctx);
  }
};

template <>
struct fmt::formatter<hmssql::ExpressionType> : formatter<string_view> {
  template <typename FormatContext>
  auto format(hmssql::ExpressionType c, FormatContext &ctx) const {
    string_view name;
    switch (c) {
      case hmssql::ExpressionType::INVALID:
        name = "Invalid";
        break;
      case hmssql::ExpressionType::CONSTANT:
        name = "Constant";
        break;
      case hmssql::ExpressionType::COLUMN_REF:
        name = "ColumnRef";
        break;
      case hmssql::ExpressionType::TYPE_CAST:
        name = "TypeCast";
        break;
      case hmssql::ExpressionType::FUNCTION:
        name = "Function";
        break;
      case hmssql::ExpressionType::AGG_CALL:
        name = "AggregationCall";
        break;
      case hmssql::ExpressionType::STAR:
        name = "Star";
        break;
      case hmssql::ExpressionType::UNARY_OP:
        name = "UnaryOperation";
        break;
      case hmssql::ExpressionType::BINARY_OP:
        name = "BinaryOperation";
        break;
      case hmssql::ExpressionType::ALIAS:
        name = "Alias";
        break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};
