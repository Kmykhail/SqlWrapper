#include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
#
#add_executable(run_tests BasicTests.cpp)
#
#target_link_libraries(run_tests gtest gtest_main)
#target_link_libraries(run_tests SqlWrapper)

#
## Locate GTest
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})
#
## Link runTests with what we want to test and the GTest and pthread library
#add_executable(runTests tests.cpp)
#target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)


## Locate GTest
#find_package(GTest REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})
#
## Link runTests with what we want to test and the GTest and pthread library
#add_executable(run_tests BasicTests.cpp)
##target_link_libraries(runTests ${GTEST_LIBRARIES} pthread)
#target_link_libraries(run_tests gtest gtest_main pthread)
#target_link_libraries(run_tests SqlWrapper)


cmake_minimum_required(VERSION 3.13)
project(run_tests)
#set(CMAKE_CXX_STANDARD 14)

find_package(PkgConfig REQUIRED)
pkg_check_modules(gtest REQUIRED gtest>=1.8.1)

SET(CMAKE_CXX_FLAGS -pthread)
SET(-Dgtest_disable_pthreads=OFF)
enable_testing()

include_directories(${gtest_INCLUDE_DIRS})

#add_executable(run_tests BasicTests.cpp ../../wrapper/SqlWrapper.h)
#target_link_libraries(run_tests ${gtest_LIBRARIES})
add_executable(run_tests BasicTests.cpp)

target_link_libraries(run_tests gtest gtest_main)
target_link_libraries(run_tests SqlWrapper)