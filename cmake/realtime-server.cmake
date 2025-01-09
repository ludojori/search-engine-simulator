include_directories(
    realtime-server/include
)

add_executable(realtimeserver
    realtime-server/src/server.cpp
    realtime-server/src/flights-provider.cpp
    utils/src/options.cpp
    utils/src/mysql-provider.cpp
)

set_target_properties(realtimeserver
    PROPERTIES
    RUNTIME_OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/realtime-server/bin/)

target_link_libraries(realtimeserver
    simple-web-server
    mysqlcppconn
)

configure_file(
    realtime-server/config.ini
    ${CMAKE_BINARY_DIR}/realtime-server/bin/
    COPYONLY
)
