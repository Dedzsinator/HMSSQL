add_library(
    hmssql_storage_table
    OBJECT
    table_heap.cpp
    table_iterator.cpp
    tuple.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:hmssql_storage_table>
    PARENT_SCOPE)
