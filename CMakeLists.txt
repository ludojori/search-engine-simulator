cmake_minimum_required(VERSION 3.0)

project(search-engine-simulator)

add_subdirectory(utils/external/Simple-Web-Server)

add_subdirectory(utils/external/popl)
include_directories(
    AFTER SYSTEM
    PUBLIC utils/external/popl/include
)

# Include API server
include(cmake/api-server.cmake)

# Include Cache Engine
include(cmake/c-engine.cmake)

# Include Real-Time Engine
include(cmake/rt-engine.cmake)
