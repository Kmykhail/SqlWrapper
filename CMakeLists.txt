cmake_minimum_required(VERSION 3.14)
project(SqlWrapper)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILE main.cpp)
add_executable(SqlWrapper_run ${SOURCE_FILE})

include_directories(wrapper)

add_subdirectory(wrapper)
add_subdirectory(SqlWrapper_test)

target_link_libraries(SqlWrapper_run SqlWrapper)