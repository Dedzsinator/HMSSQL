add_library(
  hmssql_primer
  OBJECT
  p0_trie.cpp)

set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_primer>
  PARENT_SCOPE)