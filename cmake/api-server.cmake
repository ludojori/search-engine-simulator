include_directories(
    api-server/include
)

add_executable(apiserver
    api-server/src/server.cpp
    api-server/src/options.cpp
    api-server/src/provider.cpp
)

set_target_properties(apiserver
    PROPERTIES
    RUNTIME_OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/api-server/bin/)

target_link_libraries(apiserver
    simple-web-server
    mysqlcppconn
)

configure_file(
    api-server/config.ini
    ${CMAKE_BINARY_DIR}/api-server/bin/
    COPYONLY
)