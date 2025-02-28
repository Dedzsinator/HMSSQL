

#include <iterator>
#include <memory>
#include <string>
#include "../include/binder/binder.h"
#include "../include/binder/bound_expression.h"
#include "../include/binder/bound_statement.h"
#include "../include/binder/table_ref/bound_base_table_ref.h"
#include "../include/binder/table_ref/bound_cross_product_ref.h"
#include "../include/binder/table_ref/bound_join_ref.h"
#include "../include/binder/tokens.h"
#include "../include/catalog/catalog.h"
#include "../include/common/exception.h"
#include "../include/common/util/string_util.h"
#include "fmt/format.h"
#include "fmt/ranges.h"
#include "nodes/nodes.hpp"
#include "nodes/primnodes.hpp"
#include "pg_definitions.hpp"
#include "postgres_parser.hpp"
#include "../include/type/type_id.h"

namespace hmssql {

auto Binder::BindColumnDefinition(duckdb_libpgquery::PGColumnDef *cdef) -> Column {
  std::string colname;
  if (cdef->colname != nullptr) {
    colname = cdef->colname;
  }
  if (cdef->collClause != nullptr) {
    throw NotImplementedException("coll clause on column is not supported");
  }

  auto name = std::string(
      (reinterpret_cast<duckdb_libpgquery::PGValue *>(cdef->typeName->names->tail->data.ptr_value)->val.str));

  if (name == "int4") {
    return {colname, TypeId::INTEGER};
  }

  if (name == "varchar") {
    auto exprs = BindExpressionList(cdef->typeName->typmods);
    if (exprs.size() != 1) {
      throw hmssql::Exception("should specify max length for varchar field");
    }
    const auto &varchar_max_length_val = dynamic_cast<const BoundConstant &>(*exprs[0]);
    uint32_t varchar_max_length = std::stoi(varchar_max_length_val.ToString());
    return {colname, TypeId::VARCHAR, varchar_max_length};
  }

  throw NotImplementedException(fmt::format("unsupported type: {}", name));
}

auto Binder::BindCreate(duckdb_libpgquery::PGCreateStmt *pg_stmt) -> std::unique_ptr<CreateStatement> {
  auto table = std::string(pg_stmt->relation->relname);
  auto columns = std::vector<Column>{};
  size_t column_count = 0;

  for (auto c = pg_stmt->tableElts->head; c != nullptr; c = lnext(c)) {
    auto node = reinterpret_cast<duckdb_libpgquery::PGNode *>(c->data.ptr_value);
    switch (node->type) {
      case duckdb_libpgquery::T_PGColumnDef: {
        auto cdef = reinterpret_cast<duckdb_libpgquery::PGColumnDef *>(c->data.ptr_value);
        auto centry = BindColumnDefinition(cdef);
        if (cdef->constraints != nullptr) {
          throw NotImplementedException("constraints not supported");
        }
        columns.push_back(std::move(centry));
        column_count++;
        break;
      }
      case duckdb_libpgquery::T_PGConstraint: {
        throw NotImplementedException("constraints not supported");
        break;
      }
      default:
        throw NotImplementedException("ColumnDef type not handled yet");
    }
  }

  if (column_count == 0) {
    throw hmssql::Exception("should have at least 1 column");
  }

  return std::make_unique<CreateStatement>(std::move(table), std::move(columns));
}

auto Binder::BindIndex(duckdb_libpgquery::PGIndexStmt *stmt) -> std::unique_ptr<IndexStatement> {
  std::vector<std::unique_ptr<BoundColumnRef>> cols;
  auto table = BindBaseTableRef(stmt->relation->relname, std::nullopt);

  for (auto cell = stmt->indexParams->head; cell != nullptr; cell = cell->next) {
    auto index_element = reinterpret_cast<duckdb_libpgquery::PGIndexElem *>(cell->data.ptr_value);
    if (index_element->name != nullptr) {
      auto column_ref = ResolveColumn(*table, std::vector{std::string(index_element->name)});
      cols.emplace_back(std::make_unique<BoundColumnRef>(dynamic_cast<const BoundColumnRef &>(*column_ref)));
    } else {
      throw NotImplementedException("create index by expr is not supported yet");
    }
  }

  return std::make_unique<IndexStatement>(stmt->idxname, std::move(table), std::move(cols));
}

}  // namespace hmssql
