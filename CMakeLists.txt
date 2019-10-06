cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)
project(vstruct)

include_directories(
  include
)
  
enable_testing()
find_package(GTest REQUIRED )
include_directories( ${GTEST_INCLUDE_DIRS} )

add_executable(${PROJECT_NAME}_test_ArrayClass "test/array_unit_test.cpp")
target_link_libraries(${PROJECT_NAME}_test_ArrayClass ${GTEST_BOTH_LIBRARIES} pthread)

