//===----------------------------------------------------------------------===//
//                         HMSSQL
//
// hmssql/binder/parser.h
//
//
//===----------------------------------------------------------------------===//



#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include <string>

#include "../include/binder/simplified_token.h"
#include "../include/binder/tokens.h"
#include "../include/catalog/catalog.h"
#include "../include/catalog/column.h"
#include "../include/common/macros.h"
#include "../include/common/util/string_util.h"
#include "fmt/format.h"
#include "nodes/parsenodes.hpp"
#include "nodes/pg_list.hpp"
#include "pg_definitions.hpp"
#include "postgres_parser.hpp"
#include "../include/type/type_id.h"
#include "../include/type/value.h"
#include "../include/binder/bound_statement.h"

namespace duckdb_libpgquery {
struct PGList;
struct PGSelectStmt;
struct PGAConst;
struct PGAStar;
struct PGFuncCall;
struct PGNode;
struct PGColumnRef;
struct PGResTarget;
struct PGAExpr;
struct PGJoinExpr;
}  // namespace duckdb_libpgquery

namespace hmssql {

class Catalog;
class BoundColumnRef;
class BoundExpression;
class BoundTableRef;
class BoundBaseTableRef;
class BoundExpression;
class BoundExpressionListRef;
class BoundOrderBy;
class BoundSubqueryRef;
class CreateStatement;
class ExplainStatement;
class IndexStatement;
class DeleteStatement;
class UpdateStatement;

/**
 * The binder is responsible for transforming the Postgres parse tree to a binder tree
 * which can be recognized unambiguously by the HMSSQL planner.
 */
class Binder {
 public:
  explicit Binder(const Catalog &catalog);

  auto Parse(const std::string &query) -> duckdb_libpgquery::PGNode*;

  /** Attempts to parse a query into a series of SQL statements. The parsed statements
   * will be stored in the `statements_nodes_` variable.
   */
  void ParseAndSave(const std::string &query);

  /** Return true if the given text matches a keyword of the parser. */
  static auto IsKeyword(const std::string &text) -> bool;

  /** Return a list of all keywords in the parser. */
  static auto KeywordList() -> std::vector<ParserKeyword>;

  /** Tokenize a query, returning the raw tokens together with their locations. */
  static auto Tokenize(const std::string &query) -> std::vector<SimplifiedToken>;

  /** Transform a Postgres parse tree into a std::vector of SQL Statements. */
  void SaveParseTree(duckdb_libpgquery::PGList *tree);

  /** Transform a Postgres statement into a single SQL statement. */
  auto BindStatement(duckdb_libpgquery::PGNode *stmt) -> std::unique_ptr<BoundStatement>;

  /** Get the std::string representation of a Postgres node tag. */
  static auto NodeTagToString(duckdb_libpgquery::PGNodeTag type) -> std::string;

  // The following parts are undocumented. One `BindXXX` functions simply corresponds to a
  // node type in the Postgres parse tree.

  auto BindExplain(duckdb_libpgquery::PGExplainStmt *stmt) -> std::unique_ptr<ExplainStatement>;

  auto BindCreateView(duckdb_libpgquery::PGViewStmt *pg_stmt) -> std::unique_ptr<CreateViewStatement>;

  auto ConvertQueryNodeToString(duckdb_libpgquery::PGNode *node) -> std::string;

  auto BindCreate(duckdb_libpgquery::PGCreateStmt *pg_stmt) -> std::unique_ptr<CreateStatement>;

  auto BindColumnDefinition(duckdb_libpgquery::PGColumnDef *cdef) -> Column;

  auto BindSelect(duckdb_libpgquery::PGSelectStmt *pg_stmt) -> std::unique_ptr<SelectStatement>;

  auto BindRangeSubselect(duckdb_libpgquery::PGRangeSubselect *root) -> std::unique_ptr<BoundTableRef>;

  auto BindSubquery(duckdb_libpgquery::PGSelectStmt *node, const std::string &alias)
      -> std::unique_ptr<BoundSubqueryRef>;

  auto BindSelectList(duckdb_libpgquery::PGList *list) -> std::vector<std::unique_ptr<BoundExpression>>;

  auto BindWhere(duckdb_libpgquery::PGNode *root) -> std::unique_ptr<BoundExpression>;

  auto BindGroupBy(duckdb_libpgquery::PGList *list) -> std::vector<std::unique_ptr<BoundExpression>>;

  auto BindHaving(duckdb_libpgquery::PGNode *root) -> std::unique_ptr<BoundExpression>;

  auto BindExpression(duckdb_libpgquery::PGNode *node) -> std::unique_ptr<BoundExpression>;

  auto BindExpressionList(duckdb_libpgquery::PGList *list) -> std::vector<std::unique_ptr<BoundExpression>>;

  auto BindConstant(duckdb_libpgquery::PGAConst *node) -> std::unique_ptr<BoundExpression>;

  auto BindColumnRef(duckdb_libpgquery::PGColumnRef *node) -> std::unique_ptr<BoundExpression>;

  auto BindResTarget(duckdb_libpgquery::PGResTarget *root) -> std::unique_ptr<BoundExpression>;

  auto BindStar(duckdb_libpgquery::PGAStar *node) -> std::unique_ptr<BoundExpression>;

  auto BindFuncCall(duckdb_libpgquery::PGFuncCall *root) -> std::unique_ptr<BoundExpression>;

  auto BindAExpr(duckdb_libpgquery::PGAExpr *root) -> std::unique_ptr<BoundExpression>;

  auto BindBoolExpr(duckdb_libpgquery::PGBoolExpr *root) -> std::unique_ptr<BoundExpression>;

  auto BindFrom(duckdb_libpgquery::PGList *list) -> std::unique_ptr<BoundTableRef>;

  auto BindBaseTableRef(std::string table_name, std::optional<std::string> alias) -> std::unique_ptr<BoundBaseTableRef>;

  auto BindRangeVar(duckdb_libpgquery::PGRangeVar *table_ref) -> std::unique_ptr<BoundTableRef>;

  auto BindTableRef(duckdb_libpgquery::PGNode *node) -> std::unique_ptr<BoundTableRef>;

  auto BindJoin(duckdb_libpgquery::PGJoinExpr *root) -> std::unique_ptr<BoundTableRef>;

  auto GetAllColumns(const BoundTableRef &scope) -> std::vector<std::unique_ptr<BoundExpression>>;

  auto ResolveColumn(const BoundTableRef &scope, const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundExpression>;

  auto ResolveColumnInternal(const BoundTableRef &table_ref, const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundExpression>;

  auto ResolveColumnRefFromSelectList(const std::vector<std::vector<std::string>> &subquery_select_list,
                                      const std::vector<std::string> &col_name) -> std::unique_ptr<BoundColumnRef>;

  auto ResolveColumnRefFromBaseTableRef(const BoundBaseTableRef &table_ref, const std::vector<std::string> &col_name)
      -> std::unique_ptr<BoundColumnRef>;

  auto ResolveColumnRefFromSubqueryRef(const BoundSubqueryRef &subquery_ref, const std::string &alias,
                                       const std::vector<std::string> &col_name) -> std::unique_ptr<BoundColumnRef>;

  auto BindInsert(duckdb_libpgquery::PGInsertStmt *pg_stmt) -> std::unique_ptr<InsertStatement>;

  auto BindUse(duckdb_libpgquery::PGVariableSetStmt *stmt) -> std::unique_ptr<UseStatement>;

  auto BindValuesList(duckdb_libpgquery::PGList *list) -> std::unique_ptr<BoundExpressionListRef>;

  auto BindLimitCount(duckdb_libpgquery::PGNode *root) -> std::unique_ptr<BoundExpression>;

  auto BindLimitOffset(duckdb_libpgquery::PGNode *root) -> std::unique_ptr<BoundExpression>;

  auto BindSort(duckdb_libpgquery::PGList *list) -> std::vector<std::unique_ptr<BoundOrderBy>>;

  auto BindIndex(duckdb_libpgquery::PGIndexStmt *stmt) -> std::unique_ptr<IndexStatement>;

  auto BindDelete(duckdb_libpgquery::PGDeleteStmt *stmt) -> std::unique_ptr<DeleteStatement>;

  auto BindUpdate(duckdb_libpgquery::PGUpdateStmt *stmt) -> std::unique_ptr<UpdateStatement>;

  auto BindCTE(duckdb_libpgquery::PGWithClause *node) -> std::vector<std::unique_ptr<BoundSubqueryRef>>;

  auto BindVariableSet(duckdb_libpgquery::PGVariableSetStmt *stmt) -> std::unique_ptr<VariableSetStatement>;

  auto BindVariableShow(duckdb_libpgquery::PGVariableShowStmt *stmt) -> std::unique_ptr<VariableShowStatement>;

  class ContextGuard {
   public:
    explicit ContextGuard(const BoundTableRef **scope, const CTEList **cte_scope) {
      old_scope_ = *scope;
      scope_ptr_ = scope;
      old_cte_scope_ = *cte_scope;
      cte_scope_ptr_ = cte_scope;
      *scope = nullptr;
      // do not reset CTE scope because we want to use those from parents
    }
    ~ContextGuard() {
      *scope_ptr_ = old_scope_;
      *cte_scope_ptr_ = old_cte_scope_;
    }

    DISALLOW_COPY_AND_MOVE(ContextGuard);

   private:
    const BoundTableRef *old_scope_;
    const BoundTableRef **scope_ptr_;
    const CTEList *old_cte_scope_;
    const CTEList **cte_scope_ptr_;
  };

  /** If any function needs to modify the scope, it MUST hold the context guard, so that
   * the context will be recovered after the function returns. Currently, it's used in
   * `BindFrom` and `BindJoin`.
   */
  auto NewContext() -> ContextGuard { return ContextGuard(&scope_, &cte_scope_); }

  /** Store all statement parse node */
  std::vector<duckdb_libpgquery::PGNode *> statement_nodes_;

 private:
  /** Catalog will be used during the binding process. USERS SHOULD ENSURE IT OUTLIVES THE BINDER,
   * otherwise it's a dangling reference.
   */
  const Catalog &catalog_;

  /** The current scope for resolving column ref, used in binding expressions */
  const BoundTableRef *scope_{nullptr};

  /** The current scope for resolving tables in CTEs, used in binding tables */
  const CTEList *cte_scope_{nullptr};

  /** Sometimes we will need to assign a name to some unnamed items. This variable gives them a universal ID. */
  size_t universal_id_{0};

  std::string text_;

  duckdb::PostgresParser parser_;
};

}  // namespace hmssql
