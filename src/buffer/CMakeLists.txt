add_library(
  hmssql_buffer
  OBJECT
  buffer_pool_manager_instance.cpp
  clock_replacer.cpp
  lru_replacer.cpp
  lru_k_replacer.cpp)

set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_buffer>
  PARENT_SCOPE)