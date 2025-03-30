include_directories(
    config-server/include
)

add_executable(configserver
    config-server/src/server.cpp
    config-server/src/config-provider.cpp
    utils/src/options.cpp
    utils/src/mysql-provider.cpp
)

set(BIN_DIR ${CMAKE_BINARY_DIR}/config-server/bin)

file(MAKE_DIRECTORY ${BIN_DIR})

set_target_properties(configserver
    PROPERTIES
    RUNTIME_OUTPUT_NAME server
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

target_link_libraries(configserver
    simple-web-server
    mysqlcppconn
    ValiJSON::valijson
)

configure_file(
    config-server/config.ini
    ${CMAKE_BINARY_DIR}/config-server/bin/
    COPYONLY
)

file(
    COPY ${CMAKE_SOURCE_DIR}/config-server/schemas/
    DESTINATION ${CMAKE_BINARY_DIR}/config-server/schemas
)