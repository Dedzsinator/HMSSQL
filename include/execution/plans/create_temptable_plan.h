#pragma once

#include "abstract_plan.h"

namespace hmssql {

class CreateTempTablePlanNode : public AbstractPlanNode {
 public:
  CreateTempTablePlanNode(SchemaRef output_schema, std::string table_name, std::vector<Column> columns)
      : AbstractPlanNode(std::move(output_schema), {}), table_name_(std::move(table_name)), columns_(std::move(columns)) {}

  auto GetType() const -> PlanType override { return PlanType::CreateTempTable; }

  auto CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const -> std::unique_ptr<AbstractPlanNode> override {
    return std::make_unique<CreateTempTablePlanNode>(output_schema_, table_name_, columns_);
  }

  auto GetTableName() const -> const std::string & { return table_name_; }

  auto GetColumns() const -> const std::vector<Column> & { return columns_; }

 private:
  std::string table_name_;
  std::vector<Column> columns_;

  auto PlanNodeToString() const -> std::string override {
    return fmt::format("CreateTempTablePlanNode {{ table_name: {} }}", table_name_);
  }
};

}  // namespace hmssql