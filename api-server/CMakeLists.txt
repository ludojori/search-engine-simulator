add_executable(server src/server.cpp)

add_library(options src/options.cpp)

target_link_libraries(server
    simple-web-server
    options)