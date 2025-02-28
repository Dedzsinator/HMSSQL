#pragma once

#include <string>
#include <utility>
#include <memory>
#include <vector>


#include "../include/type/value.h"
#include "../include/common/macros.h"
#include "../include/common/exception.h"
#include "fmt/ranges.h"
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

  /*
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

  /*
   * A bound aggregate call, e.g., `sum(x)`.
   */
  class BoundAggCall : public BoundExpression {
  public:
    explicit BoundAggCall(std::string func_name, bool is_distinct, std::vector<std::unique_ptr<BoundExpression>> args)
        : BoundExpression(ExpressionType::AGG_CALL),
          func_name_(std::move(func_name)),
          is_distinct_(is_distinct),
          args_(std::move(args)) {}

    auto ToString() const -> std::string override;

    auto HasAggregation() const -> bool override { return true; }

    /** Function name. */
    std::string func_name_;

    /** Is distinct aggregation */
    bool is_distinct_;

    /** Arguments of the agg call. */
    std::vector<std::unique_ptr<BoundExpression>> args_;
  };

  /*
   * The alias in SELECT list, e.g. `SELECT count(x) AS y`, the `y` is an alias.
   */
  class BoundAlias : public BoundExpression {
    public:
    explicit BoundAlias(std::string alias, std::unique_ptr<BoundExpression> child)
        : BoundExpression(ExpressionType::ALIAS), alias_(std::move(alias)), child_(std::move(child)) {}
  
    auto ToString() const -> std::string override { return fmt::format("({} as {})", child_, alias_); }
  
    auto HasAggregation() const -> bool override { return child_->HasAggregation(); }
  
    /** Alias name. */
    std::string alias_;
  
    /** The actual expression */
    std::unique_ptr<BoundExpression> child_;
  };

  /*
   * A bound binary operator, e.g., `a+b`.
   */
  class BoundBinaryOp : public BoundExpression {
    public:
    explicit BoundBinaryOp(std::string op_name, std::unique_ptr<BoundExpression> larg,
                            std::unique_ptr<BoundExpression> rarg)
        : BoundExpression(ExpressionType::BINARY_OP),
          op_name_(std::move(op_name)),
          larg_(std::move(larg)),
          rarg_(std::move(rarg)) {}
  
    auto ToString() const -> std::string override { return fmt::format("({} {} {})", larg_->ToString(), op_name_, rarg_->ToString()); }
  
    auto HasAggregation() const -> bool override { return larg_->HasAggregation() || rarg_->HasAggregation(); }
  
    /** Operator name. */
    std::string op_name_;
  
    /** Left argument of the op. */
    std::unique_ptr<BoundExpression> larg_;
  
    /** Right argument of the op. */
    std::unique_ptr<BoundExpression> rarg_;
  };

  /*
   * A bound column reference, e.g., `y.x` in the SELECT list.
   */
  class BoundColumnRef : public BoundExpression {
    public:
    explicit BoundColumnRef(std::vector<std::string> col_name)
        : BoundExpression(ExpressionType::COLUMN_REF), col_name_(std::move(col_name)) {}
  
    static auto Prepend(std::unique_ptr<BoundColumnRef> self, std::string prefix) -> std::unique_ptr<BoundColumnRef> {
      if (self == nullptr) {
        return nullptr;
      }
      std::vector<std::string> col_name{std::move(prefix)};
      std::copy(self->col_name_.cbegin(), self->col_name_.cend(), std::back_inserter(col_name));
      return std::make_unique<BoundColumnRef>(std::move(col_name));
    }
  
    auto ToString() const -> std::string override { return fmt::format("{}", fmt::join(col_name_, ".")); }
  
    auto HasAggregation() const -> bool override { return false; }
  
    /** The name of the column. */
    std::vector<std::string> col_name_;
  };

  /*
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

  /*
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

  /*
   * A bound unary operation, e.g., `-x`.
   */
  class BoundUnaryOp : public BoundExpression {
    public:
    explicit BoundUnaryOp(std::string op_name, std::unique_ptr<BoundExpression> arg)
        : BoundExpression(ExpressionType::UNARY_OP), op_name_(std::move(op_name)), arg_(std::move(arg)) {}
  
    auto ToString() const -> std::string override { return fmt::format("({}{})", op_name_, arg_); }
  
    auto HasAggregation() const -> bool override { return arg_->HasAggregation(); }
  
    /** Operator name. */
    std::string op_name_;
  
    /** Argument of the op. */
    std::unique_ptr<BoundExpression> arg_;
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
