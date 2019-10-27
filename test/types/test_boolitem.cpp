///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
///
///
///
#include <string>
#include <limits>
#include "vstruct/itemtypes.h"
#include "gtest/gtest.h"
#include "testlib.h"

namespace {

using vstruct::BoolItem;  // test target

template <uint16_t bArg>
struct TestArgs {
  enum : uint16_t {
    b = bArg
  };
};
using testing::Types;
typedef Types<
TestArgs<0>,
TestArgs<1>,
TestArgs<2>,
TestArgs<3>,
TestArgs<4>,
TestArgs<5>,
TestArgs<6>,
TestArgs<7>,
TestArgs<8>,
TestArgs<9>,
TestArgs<10>,
TestArgs<11>,
TestArgs<12>,
TestArgs<13>,
TestArgs<14>,
TestArgs<15>,
TestArgs<16>,
TestArgs<17>
> BoolItemTestArgs;

template <typename TArgs>
class TestSuite : public testing::Test
{
public:
  enum : uint16_t {
    b = TArgs::b
  };
  static const size_t pBufSize = 3;
  vstruct::pbuf_type pBufInternal_[pBufSize];
  vstruct::pbuf_type pBufExpected_[pBufSize];
  vstruct::pbuf_type* pBuf_ = {pBufInternal_};
  BoolItem<b> item{pBuf_};

  void initBuffers(vstruct::pbuf_type initial_value = 0) {
    for(size_t i=0; i < pBufSize; i++) {
      pBufInternal_[i] = initial_value;
      pBufExpected_[i] = initial_value;
    }
  }
  void setExpected(bool value) {
    uint16_t bit_pos = b;
    uint16_t byte_pos = bit_pos >> 3;
    vstruct::pbuf_type bit_mask = vstruct::pbuf_type(1) << (bit_pos % 8);
    vstruct::pbuf_type temp = pBufExpected_[byte_pos];
    if(value){
      pBufExpected_[byte_pos] = bit_mask | temp;
    } else {
      pBufExpected_[byte_pos] = (~bit_mask) & temp;
    }
  }
  bool compareBuffer() {
    bool all_equals = true;
    for(size_t i=0; i < pBufSize; i++) {
      if(pBufInternal_[i] != pBufExpected_[i]) {
        all_equals = false;
        break;
      }
    }
    return all_equals;
  }
  void checkSet(uint16_t index, bool value) {
    item = value;
    setExpected(value);
    bool check = compareBuffer();
    EXPECT_TRUE(check)
        <<"checkSet," << "index: " << index << ", value: " << value;
  }

  void checkGet(uint16_t index, bool expected) {
    bool output = item;
    EXPECT_EQ(expected, output)
        <<"checkGet";
  }
};

TYPED_TEST_SUITE_P(TestSuite);
TYPED_TEST_P(TestSuite, TestSetGet) {
  uint16_t idx = 0;
  this->initBuffers(0xff);
  this->checkSet(idx, false);
  this->checkGet(idx, false);
  this->checkSet(idx, true);
  this->checkGet(idx, true);

  this->initBuffers(0);
  this->checkSet(idx, true);
  this->checkGet(idx, true);
  this->checkSet(idx, false);
  this->checkGet(idx, false);
}


REGISTER_TYPED_TEST_SUITE_P
(
    TestSuite,
    TestSetGet
);

INSTANTIATE_TYPED_TEST_SUITE_P
(
    TestBoolItem,
    TestSuite,
    BoolItemTestArgs,
);

}  // [anonymous]

