cmake_minimum_required(VERSION 3.0)

project(search-engine-simulator)

# Include API server
include(cmake/api-server.cmake)

# Include Cache Engine
include(cmake/c-engine.cmake)

# Include Real-Time Engine
include(cmake/rt-engine.cmake)

add_subdirectory(utils/external/Simple-Web-Server)
add_subdirectory(utils/external/popl)