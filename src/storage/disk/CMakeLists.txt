add_library(
    hmssql_storage_disk 
    OBJECT
    disk_manager.cpp
    disk_manager_memory.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_storage_disk>
    PARENT_SCOPE)
