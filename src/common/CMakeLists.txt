add_library(
  hmssql_common
  OBJECT
  hmssql_instance.cpp
  config.cpp
  util/string_util.cpp)

set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_common>
  PARENT_SCOPE)