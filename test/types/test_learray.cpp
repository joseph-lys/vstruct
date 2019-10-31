///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
///
///
///
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
#include <iostream>

namespace {

using vstruct::LEArrayType;  // test target

template <typename TArg, uint16_t bitsArg, uint16_t SzArg, uint16_t NArg>
struct TestArgs {
  typedef TArg T;
  enum : uint16_t {
    bits = bitsArg,
    Sz = SzArg,
    N = NArg
  };
};

using testing::Types;
typedef Types<
TestArgs<int8_t, 0, 2, 11>,
TestArgs<int8_t, 1, 3, 11>,
TestArgs<int8_t, 7, 4, 11>,
TestArgs<int8_t, 2, 5, 11>,
TestArgs<int8_t, 3, 6, 17>,
TestArgs<int8_t, 4, 7, 9>,
TestArgs<int8_t, 0, 8, 9>,
TestArgs<int8_t, 8, 2, 9>,
TestArgs<int8_t, 9, 2, 9>,
TestArgs<int8_t, 0, 7, 11>,
TestArgs<int8_t, 1, 7, 17>,
TestArgs<int8_t, 7, 7, 18>,
TestArgs<int8_t, 8, 7, 19>,
TestArgs<int8_t, 9, 7, 23>,
TestArgs<int8_t, 0, 8, 1>,
TestArgs<int8_t, 1, 8, 3>,
TestArgs<int8_t, 7, 8, 7>,
TestArgs<int8_t, 8, 8, 8>,
TestArgs<int8_t, 9, 8, 17>,
TestArgs<int32_t, 0, 2, 14>,
TestArgs<int32_t, 1, 2, 16>,
TestArgs<int32_t, 7, 2, 18>,
TestArgs<int32_t, 8, 2, 21>,
TestArgs<int32_t, 9, 2, 25>,
TestArgs<int32_t, 0, 7, 23>,
TestArgs<int32_t, 1, 7, 23>,
TestArgs<int32_t, 7, 7, 17>,
TestArgs<int32_t, 8, 7, 17>,
TestArgs<int32_t, 9, 7, 17>,
TestArgs<int32_t, 0, 8, 17>,
TestArgs<int32_t, 1, 8, 17>,
TestArgs<int32_t, 7, 8, 17>,
TestArgs<int32_t, 8, 8, 17>,
TestArgs<int32_t, 9, 8, 17>,
TestArgs<int32_t, 1, 9, 17>,
TestArgs<int32_t, 7, 9, 17>,
TestArgs<int32_t, 8, 9, 17>,
TestArgs<int32_t, 9, 9, 17>,
TestArgs<int32_t, 13, 23, 17>,
TestArgs<int32_t, 13, 24, 17>,
TestArgs<int32_t, 13, 25, 17>,
TestArgs<int32_t, 13, 26, 17>
> LEArrayIntTestArgs;


// smoke test, some selected float types
typedef Types<
TestArgs<float, 7, 32, 7>,
TestArgs<float, 8, 32, 7>,
TestArgs<double, 15, 64, 16>,
TestArgs<double, 16, 64, 16>
> LEArraySmokeTestArgs;

template <typename TArgs>
class LEArrayTestSuite : public testing::Test {
 public:
  typedef typename TArgs::T T;
  enum : uint16_t {
    bits = TArgs::bits,
    Sz = TArgs::Sz,
    N = TArgs::N
  };
  static const size_t kBufSize = (Sz * N + 7) / 8 + 8;
  test_helpers::PackerGuess<T, Sz> packer_;
  test_helpers::RandomValue<T> random_;

  vstruct::pbuf_type pBufInternal_[kBufSize];
  vstruct::pbuf_type pBufExpected_[kBufSize];
  vstruct::pbuf_type* pBuf_ = {pBufInternal_};
  LEArrayType<T, bits, Sz, N> item{pBuf_};

  void initBuffers(vstruct::pbuf_type initial_value = 0) {
    for (size_t i=0; i < kBufSize; i++) {
      pBufInternal_[i] = initial_value;
      pBufExpected_[i] = initial_value;
    }
  }
  void checkSetGet(uint16_t index, T value) {
    T expected = packer_.expected(value);
    item[index] = value;
    T output = item[index];
      EXPECT_EQ(expected, output)
          <<"checkGetSet, value:" << value << ", type:" << typeid(output).name()<< ", bits:" << bits << ", Sz:" << Sz;
    std::cout << typeid(output).name() <<"checkGetSet,[" << index << "]=value:" << value << ", result:" << output << std::endl;
    if (expected != output) {
      item[index] = value;
      output = item[index];
    }
  }
  void checkEvenSetGet(T value) {
    for (uint16_t idx=0; idx < N; idx++) {
      if (idx % 2 == 0) {
        checkSetGet(idx, value);
      }
    }
  }
  void checkOddSetGet(T value) {
    for (uint16_t idx=0; idx < N; idx++) {
      if (idx & 1) {
        checkSetGet(idx, value);
      }
    }
  }
  void checkAllSetGet(T value) {
    for (uint16_t idx=0; idx < N; idx++) {
      checkSetGet(idx, value);
    }
  }
  void checkIncSetGet(T value, T inc) {
    for (uint16_t idx=0; idx < N; idx++) {
      value += inc;
      checkSetGet(idx, value);
    }
  }
};

TYPED_TEST_SUITE_P(LEArrayTestSuite);
TYPED_TEST_P(LEArrayTestSuite, TestSetGet) {
  auto max_value = this->packer_.maxUnpacked();
  auto min_value = this->packer_.minUnpacked();
  this->initBuffers(0xff);
  this->checkEvenSetGet(0);
  this->checkOddSetGet(0);
  this->initBuffers(0xaa);
  this->checkAllSetGet(0);
  this->initBuffers(0xaa);
  this->checkOddSetGet(min_value);
  this->checkEvenSetGet(min_value);
  this->initBuffers(0xaa);
  this->checkAllSetGet(min_value);
  this->initBuffers(0);
  this->checkOddSetGet(max_value);
  this->checkEvenSetGet(max_value);
  this->initBuffers(0xaa);
  this->checkAllSetGet(max_value);
}


TYPED_TEST_P(LEArrayTestSuite, TestSetGetOver) {
  auto max_value = std::numeric_limits<decltype(this->packer_.maxUnpacked())>::max();
  auto min_value = std::numeric_limits<decltype(this->packer_.maxUnpacked())>::min();
  this->initBuffers(0xff);
  this->checkEvenSetGet(min_value);
  this->checkOddSetGet(min_value);
  this->initBuffers(0xaa);
  this->checkAllSetGet(min_value);
  this->initBuffers(0xaa);
  this->checkOddSetGet(max_value);
  this->checkEvenSetGet(max_value);
  this->initBuffers(0xaa);
  this->checkAllSetGet(max_value);
}

TYPED_TEST_P(LEArrayTestSuite, TestFuzz) {
  decltype(this->random_.randomValue()) fuzz_value;
  decltype(this->random_.randomValue()) inc_value;
  this->initBuffers(0x00);
  for (int i=0; i < 200; i++) {
    fuzz_value = this->random_.randomValue();
    this->checkEvenSetGet(fuzz_value);
    fuzz_value = this->random_.randomValue();
    this->checkOddSetGet(fuzz_value);
    fuzz_value = this->random_.randomValue();
    this->checkAllSetGet(fuzz_value);
    fuzz_value = this->random_.randomValue();
    inc_value = this->random_.randomValue();
    this->checkIncSetGet(fuzz_value, inc_value);
  }
}


REGISTER_TYPED_TEST_SUITE_P
(
    LEArrayTestSuite,
    TestSetGet,
    TestFuzz,
    TestSetGetOver
);

INSTANTIATE_TYPED_TEST_SUITE_P
(
    TestLEArray,
    LEArrayTestSuite,
    LEArrayIntTestArgs
);
INSTANTIATE_TYPED_TEST_SUITE_P
(
    SmokeTestLEArray,
    LEArrayTestSuite,
    LEArraySmokeTestArgs
);
}  // namespace

