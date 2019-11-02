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

using TestStruct = inner_ns::outer_ns::TestStruct1;
// using TestStruct2;
// using TestStruct3;



namespace name {

// using TestStruct = test_namespace::TestGen1;
TestStruct S;

TEST(GenTest1, TestSize){
 EXPECT_EQ(1,  S.b0.elementBitSize());  // bool b0
 EXPECT_EQ(1,  S.b1.elementBitSize());  // bool b1
 EXPECT_EQ(1,  S.b2.elementBitSize());  // bool b2
 EXPECT_EQ(2,  S.x0.elementBitSize());  // uint8_t x0 : 2
 EXPECT_EQ(3,  S.x1.elementBitSize());  // int8_t x1 : 3
 EXPECT_EQ(14, S.x2.elementBitSize());  // uint16_t x2: 14
 EXPECT_EQ(15, S.x3.elementBitSize());  // int15_t x3: 15
 EXPECT_EQ(26, S.x4.elementBitSize());  // uint32_t x4 : 26
 EXPECT_EQ(27, S.x5.elementBitSize());  // int32_t x5 : 27
 EXPECT_EQ(58, S.x6.elementBitSize());  // uint64_t x6 : 58
 EXPECT_EQ(59, S.x7.elementBitSize());  // int64_t x7 : 59
 EXPECT_EQ(3 * 8 ,  S.arr0.elementBitSize());   // uint16_t arr0[4] : 3
 EXPECT_EQ(11 * 8 , S.arr0.elementBitSize());  // int16_t arr1[11] : 11
 EXPECT_EQ(16 * 8 , S.arr0.elementBitSize());  // int16_t arr2[11] : 16
 EXPECT_EQ(sizeof(float) * 8, S.f.elementBitSize());   // float flt
 EXPECT_EQ(sizeof(double) * 8, S.d.elementBitSize());  // double dbl
 EXPECT_EQ(sizeof(float) * 8 * 4, S.f.elementBitSize());   // float flt_arr[4]
 EXPECT_EQ(sizeof(double) * 8 * 4, S.d.elementBitSize());  // double dbl_arr[4]
}

TEST(GenTest1, TestPad){
 EXPECT_EQ(0, S.pad0.next_bits);
 EXPECT_EQ(0, S.pad1.next_bits % 8);
 EXPECT_EQ(S.pad1.next_bits, S.pad2.next_bits);
 EXPECT_EQ(0, S.pad3.next_bits % 16);
 EXPECT_EQ(0, S.pad4.bitSize % 32);
}

TEST(GenTest1, TestContinuity){
  EXPECT_EQ(0, S.b0.next_bits);
  EXPECT_EQ(S.b0.next_bits, S.b1.bits);
  EXPECT_EQ(S.b1.next_bits, S.b2.bits);
  EXPECT_EQ(S.b2.next_bits, S.pad0.bits);
  EXPECT_EQ(S.b1.next_bits, S.b2.bits);
}



}  // namespace
