#pragma once

#include "abstract_plan.h"

namespace hmssql {

class CreateViewPlanNode : public AbstractPlanNode {
 public:
  CreateViewPlanNode(SchemaRef output_schema, std::string view_name, std::string query)
      : AbstractPlanNode(std::move(output_schema), {}), view_name_(std::move(view_name)), query_(std::move(query)) {}

  auto GetType() const -> PlanType override { return PlanType::CreateView; }

  auto CloneWithChildren(std::vector<AbstractPlanNodeRef> children) const -> std::unique_ptr<AbstractPlanNode> override {
    return std::make_unique<CreateViewPlanNode>(output_schema_, view_name_, query_);
  }

  auto GetViewName() const -> const std::string & { return view_name_; }

  auto GetQuery() const -> const std::string & { return query_; }

 private:
  std::string view_name_;
  std::string query_;

  auto PlanNodeToString() const -> std::string override {
    return fmt::format("CreateViewPlanNode {{ view_name: {}, query: {} }}", view_name_, query_);
  }
};

}  // namespace hmssql