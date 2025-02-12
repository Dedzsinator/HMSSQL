add_library(
  hmssql_planner
  OBJECT
  expression_factory.cpp
  plan_aggregation.cpp
  plan_expression.cpp
  plan_insert.cpp
  plan_table_ref.cpp
  plan_select.cpp
  planner.cpp)

set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_planner>
  PARENT_SCOPE)