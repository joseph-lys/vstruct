///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
///
///
///

#include <vector>
#include "gtest/gtest.h"
#include "gen/example1.h"
// #include "gen/teststruct2.h"
// #include "gen/teststruct3.h"

using TestStruct = outer_ns::inner_ns::Example1;
// using TestStruct2;
// using TestStruct3;



namespace name {

// using TestStruct = test_namespace::TestGen1;
static TestStruct S;

TEST(GenTest1, TestSize){
 EXPECT_EQ(1,  S.b0.bit_size);  // bool b0
 EXPECT_EQ(1,  S.b1.bit_size);  // bool b1
 EXPECT_EQ(1,  S.b2.bit_size);  // bool b2
 EXPECT_EQ(2,  S.x0.bit_size);  // uint8_t x0 : 2
 EXPECT_EQ(3,  S.x1.bit_size);  // int8_t x1 : 3
 EXPECT_EQ(14, S.x2.bit_size);  // uint16_t x2: 14
 EXPECT_EQ(15, S.x3.bit_size);  // int15_t x3: 15
 EXPECT_EQ(26, S.x4.bit_size);  // uint32_t x4 : 26
 EXPECT_EQ(27, S.x5.bit_size);  // int32_t x5 : 27
 EXPECT_EQ(58, S.x6.bit_size);  // uint64_t x6 : 58
 EXPECT_EQ(59, S.x7.bit_size);  // int64_t x7 : 59
 EXPECT_EQ(3 * 8 ,  S.arr0.bit_size);   // uint16_t arr0[4] : 3
 EXPECT_EQ(11 * 8 , S.arr0.bit_size);  // int16_t arr1[11] : 11
 EXPECT_EQ(16 * 8 , S.arr0.bit_size);  // int16_t arr2[11] : 16
 EXPECT_EQ(sizeof(float) * 8, S.flt.bit_size);   // float flt
 EXPECT_EQ(sizeof(double) * 8, S.dbl.bit_size);  // double dbl
 EXPECT_EQ(sizeof(float) * 8 * 4, S.arr_flt.bit_size);   // float flt_arr[4]
 EXPECT_EQ(sizeof(double) * 8 * 4, S.arr_dbl.bit_size);  // double dbl_arr[4]
}

TEST(GenTest1, TestPad){
 EXPECT_EQ(0, S.pad0.next_bit);
 EXPECT_EQ(0, S.pad1.next_bit % 8);
 EXPECT_EQ(S.pad1.next_bit, S.pad2.next_bit);
 EXPECT_EQ(0, (S.pad3.next_bit - 1) % 16);
 EXPECT_EQ(0, (S.pad4.next_bit - 1) % 32);
}

TEST(GenTest1, TestContinuity){
  EXPECT_EQ(0, S.b0.next_bit);
  EXPECT_EQ(S.b0.next_bit, S.b1.bits);
  EXPECT_EQ(S.b1.next_bit, S.b2.bits);
}



}  // namespace
