include_directories(
    cache-server/include
)

add_executable(cacheserver
    cache-server/src/server.cpp
    cache-server/src/cache-provider.cpp
    utils/src/options.cpp
    utils/src/mysql-provider.cpp
)

set(BIN_DIR ${CMAKE_BINARY_DIR}/cache-server/bin)

file(MAKE_DIRECTORY ${BIN_DIR})

set_target_properties(cacheserver
    PROPERTIES
    RUNTIME_OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

target_link_libraries(cacheserver
    simple-web-server
    mysqlcppconn
)

configure_file(
    cache-server/config.ini
    ${CMAKE_BINARY_DIR}/cache-server/bin/
    COPYONLY
)