set(SHELL_SOURCES shell.cpp)
add_executable(hmssql_main ${SHELL_SOURCES})

target_link_libraries(hmssql_main hmssql hmssql_linenoise argparse utf8proc)
set_target_properties(hmssql_main PROPERTIES OUTPUT_NAME hmssql-shell)