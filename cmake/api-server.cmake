include_directories(
    api-server/include
)

add_executable(api-server
    api-server/src/server.cpp
    api-server/src/options.cpp
)

target_link_libraries(api-server
    simple-web-server
)