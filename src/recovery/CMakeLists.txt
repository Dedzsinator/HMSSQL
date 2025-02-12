add_library(
  hmssql_recovery
  OBJECT
  checkpoint_manager.cpp
  log_manager.cpp
  log_recovery.cpp)

set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_recovery>
  PARENT_SCOPE)