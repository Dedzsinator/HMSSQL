//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// hmssql/binder/sql_statement.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <utility>

#include "fmt/format.h"
#include "../common/enums/statement_type.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/binder/table_ref/bound_subquery_ref.h"
#include "../include/catalog/column.h"
#include "../include/type/value.h"
#include "../include/binder/bound_order_by.h"
#include "../include/binder/bound_table_ref.h"
#include "../include/common/enums/statement_type.h"
#include "../common/exception.h"
#include "../include/binder/tokens.h"

namespace duckdb_libpgquery {
  struct PGCreateStmt;
  struct PGCreateViewStmt;
  struct PGCreateTempTableStmt;
  }  // namespace duckdb_libpgquery

enum ExplainOptions : uint8_t {
  INVALID = 0,   /**< Default explain mode */
  BINDER = 1,    /**< Show binder results. */
  PLANNER = 2,   /**< Show planner results. */
  OPTIMIZER = 4, /**< Show optimizer results. */
  SCHEMA = 8,    /**< Show schema. */
};

namespace hmssql {

  /*
   * BoundStatement is the base class of any type of bound SQL statement.
   */
  class BoundStatement {
  public:
    explicit BoundStatement(StatementType type);
    virtual ~BoundStatement() = default;

    /** The statement type. */
    StatementType type_;

  public:
    /** Render this statement as a string. */
    virtual auto ToString() const -> std::string {
      throw Exception("ToString not supported for this type of SQLStatement");
    }
  };

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

  class CreateStatement : public BoundStatement {
    public:
    explicit CreateStatement(std::string table, std::vector<Column> columns);
  
    std::string table_;
    std::vector<Column> columns_;
  
    auto ToString() const -> std::string override;
  };
  
  class CreateViewStatement : public BoundStatement {
    public:
    explicit CreateViewStatement(std::string view, std::string query);
  
    std::string view_;
    std::string query_;
  
    auto ToString() const -> std::string override;
  };
  
  class CreateTempTableStatement : public BoundStatement {
    public:
    explicit CreateTempTableStatement(std::string table, std::vector<Column> columns);
  
    std::string table_;
    std::vector<Column> columns_;
  
    auto ToString() const -> std::string override;
  };

  class DeleteStatement : public BoundStatement {
    public:
    explicit DeleteStatement(std::unique_ptr<BoundBaseTableRef> table, std::unique_ptr<BoundExpression> expr);
  
    std::unique_ptr<BoundBaseTableRef> table_;
  
    std::unique_ptr<BoundExpression> expr_;
  
    auto ToString() const -> std::string override;
  };

  class ExplainStatement : public BoundStatement {
    public:
    explicit ExplainStatement(std::unique_ptr<BoundStatement> statement, uint8_t options);
  
    std::unique_ptr<BoundStatement> statement_;
  
    auto ToString() const -> std::string override;
  
    uint8_t options_;
  };

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

  class InsertStatement : public BoundStatement {
    public:
    explicit InsertStatement(std::unique_ptr<BoundBaseTableRef> table, std::unique_ptr<SelectStatement> select);
  
    std::unique_ptr<BoundBaseTableRef> table_;
  
    std::unique_ptr<SelectStatement> select_;
  
    auto ToString() const -> std::string override;
  };

  class Catalog;

  class SelectStatement : public BoundStatement {
  public:
    explicit SelectStatement(std::unique_ptr<BoundTableRef> table,
                            std::vector<std::unique_ptr<BoundExpression>> select_list,
                            std::unique_ptr<BoundExpression> where,
                            std::vector<std::unique_ptr<BoundExpression>> group_by,
                            std::unique_ptr<BoundExpression> having, std::unique_ptr<BoundExpression> limit_count,
                            std::unique_ptr<BoundExpression> limit_offset,
                            std::vector<std::unique_ptr<BoundOrderBy>> sort, CTEList ctes, bool is_distinct)
        : BoundStatement(StatementType::SELECT_STATEMENT),
          table_(std::move(table)),
          select_list_(std::move(select_list)),
          where_(std::move(where)),
          group_by_(std::move(group_by)),
          having_(std::move(having)),
          limit_count_(std::move(limit_count)),
          limit_offset_(std::move(limit_offset)),
          sort_(std::move(sort)),
          ctes_(std::move(ctes)),
          is_distinct_(is_distinct) {}

    /** Bound FROM clause. */
    std::unique_ptr<BoundTableRef> table_;

    /** Bound SELECT list. */
    std::vector<std::unique_ptr<BoundExpression>> select_list_;

    /** Bound WHERE clause. */
    std::unique_ptr<BoundExpression> where_;

    /** Bound GROUP BY clause. */
    std::vector<std::unique_ptr<BoundExpression>> group_by_;

    /** Bound HAVING clause. */
    std::unique_ptr<BoundExpression> having_;

    /** Bound LIMIT clause. */
    std::unique_ptr<BoundExpression> limit_count_;

    /** Bound OFFSET clause. */
    std::unique_ptr<BoundExpression> limit_offset_;

    /** Bound ORDER BY clause. */
    std::vector<std::unique_ptr<BoundOrderBy>> sort_;

    /** Bound CTE. */
    CTEList ctes_;

    /** Is SELECT DISTINCT */
    bool is_distinct_;

    auto ToString() const -> std::string override;
  };

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

  class UpdateStatement : public BoundStatement {
    public:
      explicit UpdateStatement(
          std::unique_ptr<BoundBaseTableRef> table, std::unique_ptr<BoundExpression> filter_expr,
          std::vector<std::pair<std::unique_ptr<BoundColumnRef>, std::unique_ptr<BoundExpression>>> target_expr);
    
      std::unique_ptr<BoundBaseTableRef> table_;
    
      std::unique_ptr<BoundExpression> filter_expr_;
    
      std::vector<std::pair<std::unique_ptr<BoundColumnRef>, std::unique_ptr<BoundExpression>>> target_expr_;
    
      auto ToString() const -> std::string override;
    };

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
