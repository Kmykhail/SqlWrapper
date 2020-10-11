cmake_minimum_required(VERSION 3.14)
project(SqlWrapper)

set(CMAKE_CXX_STANDARD 17)

set(SQL_CONN /usr/include/)
include_directories(${SQL_CONN})
link_directories(${SQL_CONN})

set(SQL_LIB /usr/lib)
link_directories(${SQL_LIB})

add_library(SqlWrapper SqlWrapper.cpp SqlWrapper.h)

target_link_libraries(SqlWrapper -lcurl -lmysqlcppconn)
