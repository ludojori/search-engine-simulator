include_directories(
    cache-server/include
)

add_executable(cacheserver
    cache-server/src/server.cpp
    utils/src/options.cpp
    utils/src/mysql-provider.cpp
)

set_target_properties(cacheserver
    PROPERTIES
    RUNTIME_OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/cache-server/bin/)

target_link_libraries(cacheserver
    simple-web-server
    mysqlcppconn
)
