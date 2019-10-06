#include <iostream>
#include <iomanip>
#include <array>
#include "vstruct.h"

#include "gtest/gtest.h"
namespace{

using testing::Types;

template <typename Targ, Szarg, Narg>
struct TestArgs
{
  typdef typename Targ T; 
  enum : uint16_t { Sz = Szarg }
  enum : size_t { N = Narg }
}

typedef Types<
  TestArgs<bool, 1, 8>,
  TestArgs<uint8_t, 7, 8>,
    TestArgs<int8_t, 7, 8>,
  TestArgs<uint16_t, 7, 8>,
    TestArgs<uint16_t, 15, 8>,
  TestArgs<int16_t, 7, 8>,
    TestArgs<int16_t, 15, 8>,
  TestArgs<uint32_t, 7, 8>,
    TestArgs<uint32_t, 15, 8>,
    TestArgs<uint32_t, 23, 8>,
    TestArgs<uint32_t, 31, 8>,
  TestArgs<int32_t, 7, 8>,
    TestArgs<int32_t, 15, 8>,
    TestArgs<int32_t, 23, 8>,
    TestArgs<int32_t, 31, 8>
    > Implementations;



template <class T, size16_t Sz, size_t N>
class ArrayTest: public testing::Test {
public:
  typename Type
  class TestStruct vstruct::VStruct
  {
      vstruct::Array<bool, 1>  b0{*this, 8};
      vstruct::Array<
        typename TypeParam::T, 
        TypeParam::Sz> x{*this, N};
      vstruct::Array<bool, 1>  b1{*this, 8};
  } ;
  
  const size_t N_;
  TestStruct test_struct_;
  uint8_t buffer_[2 + (((Sz * N) + 7) >> 3)];
  
  VStruct():N_{N}, test_struct_{}, arr_{0}{ test_struct_.setBuffer(buffer_); }

}

TYPED_TEST_SUITE_P(ArrayTest); 

TYPED_TEST_P(ArrayTest, )

REGISTER_TYPED_TEST_SUITE_P
(
ArrayTest, // test suite name
// rest are testcases

);

INSTANTIATE_TYPED_TEST_SUITE_P(ArrayTest, ArrayTestCase, )
