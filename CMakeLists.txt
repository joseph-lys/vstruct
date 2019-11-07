cmake_minimum_required(VERSION 2.8)
add_compile_options(-std=c++11)
project(vstruct)

# set(CMAKE_CXX_CPPLINT "cpplint;--linelength=120")
execute_process(
  COMMAND cpplint include/*.h include/**/*.h
)

include_directories(
  include
)


# unit tests
enable_testing()
find_package(GTest REQUIRED )
include_directories( ${GTEST_INCLUDE_DIRS} )

add_executable(   # test core algo
${PROJECT_NAME}_test_internal
  "test/main.cpp"
  "test/internal/test_packer.cpp"
  "test/internal/test_leorder.cpp")
target_include_directories(${PROJECT_NAME}_test_internal PRIVATE test test/internal) # additional headers to test templated types
target_link_libraries(${PROJECT_NAME}_test_internal ${GTEST_BOTH_LIBRARIES} pthread)

add_executable(  # test types
${PROJECT_NAME}_test_types
  "test/main.cpp"
  "test/types/test_leitem.cpp"
  "test/types/test_boolitem.cpp"
  "test/types/test_learray.cpp"
  "test/types/test_boolarray.cpp"
  "test/types/test_alignpad.cpp"
  "test/types/test_functions.cpp")
target_include_directories(${PROJECT_NAME}_test_types PRIVATE test test/types) # additional headers to test templated types
target_link_libraries(${PROJECT_NAME}_test_types ${GTEST_BOTH_LIBRARIES} pthread)

# test a generated structs
add_custom_target(
    install_python_script ALL
  COMMAND
    pip3 install -e .
  WORKING_DIRECTORY
    ${PROJECT_SOURCE_DIR}/py_src
)
add_custom_target(
  generated_headers ALL
  COMMAND
    vstruct_gen_header.py -f example1.py -o gen/example1.h -n outer_ns inner_ns
  WORKING_DIRECTORY
    ${PROJECT_SOURCE_DIR}/test/generated
  BYPRODUCTS ${PROJECT_SOURCE_DIR}/test/generated/gen/example1.h
  COMMENT "generating example1.h"
)
add_executable(
${PROJECT_NAME}_test_generated
  "test/main.cpp"
  "test/generated/test_generated.cpp")
add_dependencies(generated_headers install_python_script)
add_dependencies(${PROJECT_NAME}_test_generated generated_headers)

target_include_directories(${PROJECT_NAME}_test_generated PRIVATE test/generated/gen) # additional headers
target_link_libraries(${PROJECT_NAME}_test_generated ${GTEST_BOTH_LIBRARIES} pthread)

add_test(${PROJECT_NAME}_test_internal ${PROJECT_NAME}_test_internal)
add_test(${PROJECT_NAME}_test_types ${PROJECT_NAME}_test_types)


# examples
add_executable(${PROJECT_NAME}_vstruct_example "examples/vstruct_example.cpp")
target_include_directories(${PROJECT_NAME}_vstruct_example PRIVATE examples) # additional headers

