//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// executor_context.h
//
// Identification: src/include/execution/executor_context.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <unordered_set>
#include <utility>
#include <vector>

#include "../include/catalog/catalog.h"
#include "../include/storage/page/tmp_tuple_page.h"

namespace hmssql {
/**
 * ExecutorContext stores all the context necessary to run an executor.
 */
class ExecutorContext {
 public:
  /**
   * Creates an ExecutorContext for the transaction that is executing the query.
   * @param catalog The catalog that the executor uses
   * @param bpm The buffer pool manager that the executor uses
   */
  ExecutorContext(Catalog *catalog, BufferPoolManager *bpm): catalog_{catalog}, bpm_{bpm} {}

  ~ExecutorContext() = default;

  DISALLOW_COPY_AND_MOVE(ExecutorContext);

  /** @return the catalog */
  auto GetCatalog() -> Catalog * { return catalog_; }

  /** @return the buffer pool manager */
  auto GetBufferPoolManager() -> BufferPoolManager * { return bpm_; }

  /** @return the log manager - don't worry about it for now */
  auto GetLogManager() -> LogManager * { return nullptr; }

 private:
  /** The datbase catalog associated with this executor context */
  Catalog *catalog_;
  /** The buffer pool manager associated with this executor context */
  BufferPoolManager *bpm_;
};

}  // namespace hmssql
