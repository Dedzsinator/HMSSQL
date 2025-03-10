#pragma once

#include <optional>
#include <string>
#include <utility>
#include "../include/binder/bound_table_ref.h"
#include "../include/catalog/schema.h"
#include "../include/catalog/catalog.h" // Include the catalog header to get the TableInfo type
#include "fmt/core.h"

namespace hmssql {

/**
 * A bound table ref type for single table. e.g., `SELECT x FROM y`, where `y` is `BoundBaseTableRef`.
 */
class BoundBaseTableRef : public BoundTableRef {
 public:
  explicit BoundBaseTableRef(std::string table, table_oid_t oid, std::optional<std::string> alias, Schema schema)
      : BoundTableRef(TableReferenceType::BASE_TABLE),
        table_(std::move(table)),
        oid_(oid),
        alias_(std::move(alias)),
        schema_(std::move(schema)) {}

  explicit BoundBaseTableRef(TableInfo *table_info)
      : BoundBaseTableRef(table_info->name_, table_info->oid_, std::nullopt, table_info->schema_) {
    table_info_ = table_info;
  }

  auto ToString() const -> std::string override {
    if (alias_ == std::nullopt) {
      return fmt::format("BoundBaseTableRef {{ table={}, oid={} }}", table_, oid_);
    }
    return fmt::format("BoundBaseTableRef {{ table={}, oid={}, alias={} }}", table_, oid_, *alias_);
  }

  auto GetBoundTableName() const -> std::string {
    if (alias_ != std::nullopt) {
      return *alias_;
    }
    return table_;
  }

  /** The name of the table. */
  std::string table_;

  /** The oid of the table. */
  table_oid_t oid_;

  /** The alias of the table */
  std::optional<std::string> alias_;

  /** The schema of the table. */
  Schema schema_;

 private:
  TableInfo *table_info_;
};

}  // namespace hmssql