//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// executor_factory.h
//
// Identification: src/include/execution/executor_factory.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <memory>

#include "../include/execution/executors/abstract_executor.h"
#include "../include/execution/plans/abstract_plan.h"

namespace hmssql {
/**
 * ExecutorFactory creates executors for arbitrary plan nodes.
 */
class ExecutorFactory {
 public:
  /**
   * Creates a new executor given the executor context and plan node.
   * @param exec_ctx The executor context for the created executor
   * @param plan The plan node that needs to be executed
   * @return An executor for the given plan in the provided context
   */
  static auto CreateExecutor(ExecutorContext *exec_ctx, const AbstractPlanNodeRef &plan)
      -> std::unique_ptr<AbstractExecutor>;
};
}  // namespace hmssql
