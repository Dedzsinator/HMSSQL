//===----------------------------------------------------------------------===//
//
//                         HMSSQL
//
// execution_engine.h
//
// Identification: src/include/execution/execution_engine.h
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include <vector>

#include "../include/buffer/buffer_pool_manager.h"
#include "../include/catalog/catalog.h"
#include "../include/execution/executor_context.h"
#include "../include/execution/executor_factory.h"
#include "../include/execution/plans/abstract_plan.h"
#include "../include/storage/table/tuple.h"

namespace hmssql {

/**
 * The ExecutionEngine class executes query plans.
 */
class ExecutionEngine {
 public:
  /**
   * Construct a new ExecutionEngine instance.
   * @param bpm The buffer pool manager used by the execution engine
   * @param catalog The catalog used by the execution engine
   */
  ExecutionEngine(BufferPoolManager *bpm, Catalog *catalog)
      : bpm_{bpm}, catalog_{catalog} {}

  DISALLOW_COPY_AND_MOVE(ExecutionEngine);

  /**
   * Execute a query plan.
   * @param plan The query plan to execute
   * @param result_set The set of tuples produced by executing the plan
   * @param exec_ctx The executor context in which the query executes
   * @return `true` if execution of the query plan succeeds, `false` otherwise
   */
  // NOLINTNEXTLINE
  auto Execute(const AbstractPlanNodeRef &plan, std::vector<Tuple> *result_set,
               ExecutorContext *exec_ctx) -> bool {

    // Construct the executor for the abstract plan node
    auto executor = ExecutorFactory::CreateExecutor(exec_ctx, plan);

    // Initialize the executor
    auto executor_succeeded = true;

    try {
      executor->Init();
      PollExecutor(executor.get(), plan, result_set);
    } catch (const ExecutionException &ex) {
      //"Error Encountered in Executor Execution: %s", ex.what());
      executor_succeeded = false;
      if (result_set != nullptr) {
        result_set->clear();
      }
    }

    return executor_succeeded;
  }

  void ExecuteCreateView(const CreateViewStatement &stmt);
  void ExecuteCreateTempTable(const CreateTempTableStatement &stmt);

 private:
  /**
   * Poll the executor until exhausted, or exception escapes.
   * @param executor The root executor
   * @param plan The plan to execute
   * @param result_set The tuple result set
   */
  static void PollExecutor(AbstractExecutor *executor, const AbstractPlanNodeRef &plan,
                           std::vector<Tuple> *result_set) {
    RID rid{};
    Tuple tuple{};
    while (executor->Next(&tuple, &rid)) {
      if (result_set != nullptr) {
        result_set->push_back(tuple);
      }
    }
  }

  [[maybe_unused]] BufferPoolManager *bpm_;
  [[maybe_unused]] Catalog *catalog_;
};

}  // namespace hmssql
