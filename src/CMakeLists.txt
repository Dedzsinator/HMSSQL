add_subdirectory(binder)
add_subdirectory(buffer)
add_subdirectory(catalog)
add_subdirectory(common)
add_subdirectory(concurrency)
add_subdirectory(container)
add_subdirectory(execution)
add_subdirectory(recovery)
add_subdirectory(storage)
add_subdirectory(type)
add_subdirectory(planner)
add_subdirectory(primer)
add_subdirectory(optimizer)

add_library(hmssql STATIC ${ALL_OBJECT_FILES})

set(BUSTUB_LIBS
        hmssql_binder
        hmssql_buffer
        hmssql_catalog
        hmssql_common
        hmssql_concurrency
        hmssql_execution
        hmssql_recovery
        hmssql_type
        hmssql_container_hash
        hmssql_container_disk_hash
        hmssql_storage_disk
        hmssql_storage_index
        hmssql_storage_page
        hmssql_storage_table
        hmssql_planner
        hmssql_primer
        hmssql_optimizer
        )

find_package(Threads REQUIRED)

set(BUSTUB_THIRDPARTY_LIBS
        hmssql_murmur3
        duckdb_pg_query
        fmt
        libfort::fort
        Threads::Threads
        )

target_link_libraries(
        hmssql
        ${BUSTUB_LIBS}
        ${BUSTUB_THIRDPARTY_LIBS})

target_include_directories(
        hmssql PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)