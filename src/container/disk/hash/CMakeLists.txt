add_library(
  hmssql_container_disk_hash
  OBJECT
        disk_extendible_hash_table.cpp
        linear_probe_hash_table.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_container_disk_hash>
    PARENT_SCOPE)
