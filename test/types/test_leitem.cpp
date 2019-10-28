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
#include <typeinfo>
#include "vstruct/itemtypes.h"
#include "gtest/gtest.h"
#include "../testlib.h"

namespace {

using vstruct::LEItem;  // test target

template <typename TArg, uint16_t bitsArg, uint16_t SzArg>
struct TestArgs {
  typedef TArg T;
  enum : uint16_t {
    bits = bitsArg,
    Sz = SzArg
  };
};

using testing::Types;
typedef Types<
TestArgs<int8_t, 0, 2>,
TestArgs<int8_t, 1, 2>,
TestArgs<int8_t, 7, 2>,
TestArgs<int8_t, 8, 2>,
TestArgs<int8_t, 9, 2>,
TestArgs<int8_t, 0, 7>,
TestArgs<int8_t, 1, 7>,
TestArgs<int8_t, 7, 7>,
TestArgs<int8_t, 8, 7>,
TestArgs<int8_t, 9, 7>,
TestArgs<int8_t, 0, 8>,
TestArgs<int8_t, 1, 8>,
TestArgs<int8_t, 7, 8>,
TestArgs<int8_t, 8, 8>,
TestArgs<int8_t, 9, 8>,
TestArgs<int32_t, 0, 2>,
TestArgs<int32_t, 1, 2>,
TestArgs<int32_t, 7, 2>,
TestArgs<int32_t, 8, 2>,
TestArgs<int32_t, 9, 2>,
TestArgs<int32_t, 0, 7>,
TestArgs<int32_t, 1, 7>,
TestArgs<int32_t, 7, 7>,
TestArgs<int32_t, 8, 7>,
TestArgs<int32_t, 9, 7>,
TestArgs<int32_t, 0, 8>,
TestArgs<int32_t, 1, 8>,
TestArgs<int32_t, 7, 8>,
TestArgs<int32_t, 8, 8>,
TestArgs<int32_t, 9, 8>,
TestArgs<int32_t, 1, 9>,
TestArgs<int32_t, 7, 9>,
TestArgs<int32_t, 8, 9>,
TestArgs<int32_t, 9, 9>,
TestArgs<int32_t, 13, 23>,
TestArgs<int32_t, 13, 24>,
TestArgs<int32_t, 13, 25>,
TestArgs<int32_t, 13, 26>,

// smoke test, some selected float types
TestArgs<uint8_t, 13, 7>,
TestArgs<uint16_t, 15, 15>,
TestArgs<int16_t, 15, 15>,
TestArgs<uint32_t, 7, 31>,
TestArgs<int32_t, 8, 32>,
TestArgs<uint64_t, 4, 64>,
TestArgs<int64_t, 3, 63>
> LEItemIntTestArgs;


// smoke test, some selected float types
typedef Types<
TestArgs<float, 7, 32>,
TestArgs<float, 8, 32>,
TestArgs<double, 15, 64>,
TestArgs<double, 16, 64>
> LEItemSmokeTestArgs;

template <typename TArgs>
class LEItemTestSuite : public testing::Test {
 public:
  typedef typename TArgs::T T;
  enum : uint16_t {
    bits = TArgs::bits,
    Sz = TArgs::Sz
  };
  static const size_t kBufSize = 12;
  test_helpers::PackerGuess<T, Sz> packer_;
  test_helpers::RandomValue<T> random_;

  vstruct::pbuf_type pBufInternal_[kBufSize];
  vstruct::pbuf_type pBufExpected_[kBufSize];
  vstruct::pbuf_type* pBuf_ = {pBufInternal_};
  LEItem<T, bits, Sz> item{pBuf_};

  void initBuffers(vstruct::pbuf_type initial_value = 0) {
    for (size_t i=0; i < kBufSize; i++) {
      pBufInternal_[i] = initial_value;
      pBufExpected_[i] = initial_value;
    }
  }
  void checkSetGet(typename TArgs::T value) {
    T expected = packer_.expected(value);
    item = value;
    T output = item;
      EXPECT_EQ(expected, output)
          <<"checkGetSet, value:" << value << ", type:" << typeid(output).name()<< ", bits:" << bits << ", Sz:" << Sz;
    if (expected != output) {
      item = value;
      output = item;
    }
  }
};

TYPED_TEST_SUITE_P(LEItemTestSuite);
TYPED_TEST_P(LEItemTestSuite, TestSetGet) {
  auto max_value = this->packer_.maxUnpacked();
  auto min_value = this->packer_.minUnpacked();
  this->initBuffers(0xff);
  this->checkSetGet(0);
  this->initBuffers(0xaa);
  this->checkSetGet(min_value);
  this->initBuffers(0);
  this->checkSetGet(max_value);
}


TYPED_TEST_P(LEItemTestSuite, TestFuzz) {
  decltype(this->random_.randomValue()) fuzz_value;
  this->initBuffers(0xaa);
  for (int i=0; i < 200; i++) {
    fuzz_value = this->random_.randomValue();
    this->checkSetGet(fuzz_value);
  }
}


REGISTER_TYPED_TEST_SUITE_P
(
    LEItemTestSuite,
    TestSetGet,
    TestFuzz
);

INSTANTIATE_TYPED_TEST_SUITE_P
(
    TestLEItem,
    LEItemTestSuite,
    LEItemIntTestArgs
);
INSTANTIATE_TYPED_TEST_SUITE_P
(
    SmokeTestLEItem,
    LEItemTestSuite,
    LEItemSmokeTestArgs
);
}  // namespace

