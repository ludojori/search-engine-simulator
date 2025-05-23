cmake_minimum_required(VERSION 3.0)

project(search-engine-simulator VERSION 1.0.0)

# Find Boost
set(Boost_NO_SYSTEM_PATHS ON)  # Ignore system-wide Boost installations
set(BOOST_ROOT "/opt/boost_1_75_0")  # Path to your Boost installation
set(BOOST_INCLUDEDIR "/opt/boost_1_75_0/include")
set(BOOST_LIBRARYDIR "/opt/boost_1_75_0/lib")

find_package(Boost REQUIRED COMPONENTS system filesystem)  # List required components

if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    link_directories(${Boost_LIBRARY_DIRS})
else ()
    message(FATAL_ERROR "Boost not found!")
endif ()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Updating submodules for this project...")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules.")
        endif()
        message(STATUS "Done.")
    endif()
else()
    message(STATUS "Git is required to build this project!")
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/utils/external/Simple-Web-Server/CMakeLists.txt")
    message(FATAL_ERROR "Missing CMakeLists.txt file for submodule Simple-Web-Server!")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/utils/external/valijson/CMakeLists.txt")
    message(FATAL_ERROR "Missing CMakeLists.txt file for submodule valijson!")
endif()
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/utils/external/popl/CMakeLists.txt")
    message(FATAL_ERROR "Missing CMakeLists.txt file for submodule popl!")
endif()

set(valijson_BUILD_TESTS OFF CACHE BOOL "Don't build valijson tests.")

add_subdirectory(utils/external/Simple-Web-Server EXCLUDE_FROM_ALL)
add_subdirectory(utils/external/valijson EXCLUDE_FROM_ALL)
add_subdirectory(utils/external/Catch2 EXCLUDE_FROM_ALL)
add_subdirectory(utils/external/mysql-connector-cpp EXCLUDE_FROM_ALL)

include_directories(
    utils/include
    utils/external/popl/include
    utils/external/valijson/include
    utils/external/mysql-connector-cpp/include
)

# Include Config server
include(cmake/config-server.cmake)

# Include Cache Server
include(cmake/cache-server.cmake)

# Include Real-Time Server
include(cmake/realtime-server.cmake)
