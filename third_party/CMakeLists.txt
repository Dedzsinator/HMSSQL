add_subdirectory(murmur3)
add_subdirectory(libpg_query)

add_subdirectory(fmt)
add_subdirectory(linenoise)

set(FORT_ENABLE_TESTING OFF CACHE INTERNAL "")
add_subdirectory(libfort)

add_subdirectory(argparse)

add_subdirectory(utf8proc)
