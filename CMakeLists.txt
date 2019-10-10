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


add_executable(${PROJECT_NAME}_test_Packer "test/packer_test.cpp")
target_link_libraries(${PROJECT_NAME}_test_Packer ${GTEST_BOTH_LIBRARIES} pthread)


add_executable(${PROJECT_NAME}_test_RawIF "test/raw_if_test.cpp")
target_link_libraries(${PROJECT_NAME}_test_RawIF ${GTEST_BOTH_LIBRARIES} pthread)

# examples
file(GLOB examples
    "examples/*.h"
    "examples/*.cpp"
)
add_executable(${PROJECT_NAME}_examples ${examples})
