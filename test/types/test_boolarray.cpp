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
#include "./testlib.h"

namespace {

using vstruct::BoolArrayType;  // test target

template <uint16_t bArg, uint16_t NArg>
struct TestArgs {
  enum : uint16_t {
    b = bArg,
    N = NArg
  };
};
using testing::Types;
typedef Types<
TestArgs<0, 1>,
TestArgs<0, 2>,
TestArgs<0, 3>,
TestArgs<0, 4>,
TestArgs<0, 5>,
TestArgs<0, 6>,
TestArgs<0, 7>,
TestArgs<0, 8>,
TestArgs<1, 1>,
TestArgs<2, 3>,
TestArgs<3, 5>,
TestArgs<4, 7>,
TestArgs<5, 8>,
TestArgs<6, 11>,
TestArgs<7, 13>,
TestArgs<8, 15>,
TestArgs<9, 17>,
TestArgs<10, 18>,
TestArgs<11, 12>,
TestArgs<12, 20>,
TestArgs<13, 14>,
TestArgs<14, 22>,
TestArgs<15, 24>,
TestArgs<16, 16>,
TestArgs<17, 18>,
TestArgs<18, 20>
> BoolArrayTestArgs;

template <typename TArgs>
class BoolArrayTestSuite : public testing::Test {
 public:
  enum : uint16_t {
    b = TArgs::b,
    N = TArgs::N
  };
  static const size_t kBufSize = 10;
  vstruct::pbuf_type pBufInternal_[kBufSize];
  vstruct::pbuf_type pBufExpected_[kBufSize];
  vstruct::pbuf_type* pBuf_ = {pBufInternal_};
  BoolArrayType<b, N> item{pBuf_};

  void initBuffers(vstruct::pbuf_type initial_value = 0) {
    for (size_t i=0; i < kBufSize; i++) {
      pBufInternal_[i] = initial_value;
      pBufExpected_[i] = initial_value;
    }
  }
  void setExpected(uint16_t index, bool value) {
    uint16_t bit_pos = (index + b);
    uint16_t byte_pos = bit_pos >> 3;
    vstruct::pbuf_type bit_mask = vstruct::pbuf_type(1) << (bit_pos % 8);
    vstruct::pbuf_type temp = pBufExpected_[byte_pos];
    if (value) {
      pBufExpected_[byte_pos] = bit_mask | temp;
    } else {
      pBufExpected_[byte_pos] = (~bit_mask) & temp;
    }
  }
  bool compareBuffer() {
    bool all_equals = true;
    for (size_t i=0; i < kBufSize; i++) {
      if (pBufInternal_[i] != pBufExpected_[i]) {
        all_equals = false;
        break;
      }
    }
    return all_equals;
  }
  void checkSet(uint16_t index, bool value) {
    item[index] = value;
    setExpected(index, value);
    bool check = compareBuffer();
    EXPECT_TRUE(check)
        <<"checkSet," << "index: " << index << ", value: " << value;
  }

  void checkGet(uint16_t index, bool expected) {
    bool output = item[index];
    EXPECT_EQ(expected, output)
        <<"checkGet";
  }
};

TYPED_TEST_CASE_P(BoolArrayTestSuite);
TYPED_TEST_P(BoolArrayTestSuite, TestFirst) {
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

TYPED_TEST_P(BoolArrayTestSuite, TestSecond) {
  if (this->N > 1) {
    uint16_t idx = 1;
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
}
TYPED_TEST_P(BoolArrayTestSuite, TestLast) {
  if (this->N > 2) {
    uint16_t idx = this->N - 1;
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
}
TYPED_TEST_P(BoolArrayTestSuite, TestSecondLast) {
  if (this->N > 3) {
    uint16_t idx = this->N - 2;
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
}

REGISTER_TYPED_TEST_CASE_P
(
    BoolArrayTestSuite,
    TestFirst,
    TestSecond,
    TestLast,
    TestSecondLast
);

INSTANTIATE_TYPED_TEST_CASE_P
(
    TestBoolArray,
    BoolArrayTestSuite,
    BoolArrayTestArgs
);

}  // namespace

