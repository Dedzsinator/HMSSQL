add_library(
  hmssql_concurrency
  OBJECT
  lock_manager.cpp
  transaction_manager.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_concurrency>
    PARENT_SCOPE)
