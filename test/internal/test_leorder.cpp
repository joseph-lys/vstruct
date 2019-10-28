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
#include "vstruct/internals.h"
#include "gtest/gtest.h"
#include "testlib.h"

namespace {

template <typename TypeArg, uint16_t offsetArg, uint16_t SzArg>
struct TestArgs {
  typedef TypeArg T;
  enum : uint16_t {
    offset = offsetArg,
    Sz = SzArg
  };
};

using testing::Types;
typedef Types<
TestArgs<uint8_t, 0, 2>,
TestArgs<uint8_t, 1, 2>,
TestArgs<uint8_t, 6, 2>,
TestArgs<uint8_t, 7, 2>,
TestArgs<uint8_t, 0, 7>,
TestArgs<uint8_t, 3, 7>,
TestArgs<uint8_t, 5, 7>,
TestArgs<uint8_t, 7, 7>,
TestArgs<uint8_t, 0, 8>,
TestArgs<uint8_t, 1, 8>,
TestArgs<uint8_t, 6, 8>,
TestArgs<uint8_t, 7, 8>,
TestArgs<uint16_t, 0, 3>,
TestArgs<uint16_t, 3, 3>,
TestArgs<uint16_t, 6, 3>,
TestArgs<uint16_t, 7, 3>,
TestArgs<uint16_t, 0, 15>,
TestArgs<uint16_t, 1, 15>,
TestArgs<uint16_t, 7, 15>,
TestArgs<uint16_t, 0, 16>,
TestArgs<uint16_t, 1, 16>,
TestArgs<uint16_t, 7, 16>,
TestArgs<uint32_t, 0, 3>,
TestArgs<uint32_t, 2, 3>,
TestArgs<uint32_t, 0, 5>,
TestArgs<uint32_t, 2, 5>,
TestArgs<uint32_t, 0, 31>,
TestArgs<uint32_t, 1, 31>,
TestArgs<uint32_t, 7, 31>,
TestArgs<uint32_t, 0, 32>,
TestArgs<uint32_t, 2, 32>,
TestArgs<uint32_t, 5, 32>,
TestArgs<uint32_t, 7, 32>,
TestArgs<uint64_t, 0, 16>,
TestArgs<uint64_t, 1, 16>,
TestArgs<uint64_t, 7, 16>,
TestArgs<uint64_t, 0, 17>,
TestArgs<uint64_t, 2, 17>,
TestArgs<uint64_t, 6, 17>,
TestArgs<uint64_t, 1, 1>,
TestArgs<uint64_t, 3, 3>,
TestArgs<uint64_t, 7, 7>,
TestArgs<uint64_t, 0, 64>,
TestArgs<uint64_t, 1, 64>,
TestArgs<uint64_t, 3, 64>,
TestArgs<uint64_t, 4, 64>,
TestArgs<uint64_t, 6, 64>,
TestArgs<uint64_t, 7, 64>
> AllTestArgs;


using test_helpers::PackerGuess;
using test_helpers::CodeGen;
template <typename TArgs>
class LEOrderTestSuite : public testing::Test
{
public:
  typedef typename TArgs::T T;
  static_assert(std::is_unsigned<T>::value || !std::is_integral<T>::value,
                "Only unsigned interger types allowed");
  enum : uint16_t {
    offset = TArgs::offset,
    Sz = TArgs::Sz
  };
  static_assert(sizeof(T) * 8 >= Sz,
                "Sz must be less than or equal to bit size of T");

  typedef unsigned __int128 bigint;
  vstruct::pbuf_type pbuf[16];

  bigint fromBuffer(){
    bigint x = 0;
    for(int i = 0; i < 16; i++) {
      x <<= 1;
      x |= pbuf[i];
    }
    return x;
  }
  void toBuffer(bigint x){
    for(int i = 0; i < 16; i++) {
      pbuf[i] = (x >> (i * 8)) & 0xff;
    }
  }

  void checkSet(T value, const char debug_str[])
  {
    toBuffer(0);

    vstruct::internals::LEOrder<T, offset, Sz>::set(pbuf, value);
    bigint output = fromBuffer();
    bigint expected = static_cast<bigint>(value) << offset;

    uint64_t output64, expected64; // split 128 bits to two halves checks
    output64 = static_cast<uint64_t>(output & uint64_t(0xffffffffffffffff));
    expected64 = static_cast<uint64_t>(output & uint64_t(0xffffffffffffffff));
    EXPECT_EQ(output64, expected64)
        << "set (lower 64 bits), "
        << debug_str
        << ", value:" << static_cast<uint64_t>(value) << " Sz:" << Sz
        << " offset:" << offset
        << " max:" << static_cast<uint64_t>(PackerGuess<T,Sz>::maxPacked())
        << " min:" << static_cast<uint64_t>(PackerGuess<T,Sz>::minPacked());
    if(output64 != expected64) {
      vstruct::internals::LEOrder<T, offset, Sz>::set(pbuf, 0);
    }
    output64 = static_cast<uint64_t>(output >> 64);
    expected64 = static_cast<uint64_t>(output >> 64);
    EXPECT_EQ(output64, expected64)
        << "set (upper 64 bits), "
        << debug_str
        << ", value:" << static_cast<uint64_t>(value) << " Sz:" << Sz
        << " offset:" << offset
        << " max:" << static_cast<uint64_t>(PackerGuess<T,Sz>::maxPacked())
        << " min:" << static_cast<uint64_t>(PackerGuess<T,Sz>::minPacked());
    if(output64 != expected64) {
      vstruct::internals::LEOrder<T, offset, Sz>::set(pbuf, 0);
    }
    // clear to 0
    vstruct::internals::LEOrder<T, offset, Sz>::set(pbuf, 0);
    output = fromBuffer();
    expected = 0;
    EXPECT_EQ(output, expected)
        << "clear, "
        << debug_str
        << ", value:" << static_cast<uint64_t>(value) << " Sz:" << Sz
        << " offset:" << offset
        << " max:" << static_cast<uint64_t>(PackerGuess<T,Sz>::maxPacked())
        << " min:" << static_cast<uint64_t>(PackerGuess<T,Sz>::minPacked());
    if(output != expected) {
      vstruct::internals::LEOrder<T, offset, Sz>::set(pbuf, 0);
    }
  }

  void checkGet(T value, const char debug_str[])
  {
    bigint x = static_cast<bigint>(value) << offset;
    toBuffer(x);
    T output = vstruct::internals::LEOrder<T, offset, Sz>::get(pbuf);
    T expected = value;

    uint64_t output64, expected64; // split 128 bits to two halves checks

    EXPECT_EQ(output, expected)
        << "get, "
        << debug_str
        << ", value:" << static_cast<uint64_t>(value) << " Sz:" << Sz
        << " offset:" << offset
        << " max:" << static_cast<uint64_t>(PackerGuess<T,Sz>::maxPacked())
        << " min:" << static_cast<uint64_t>(PackerGuess<T,Sz>::minPacked());
    if(output != expected) {
      volatile T temp = vstruct::internals::LEOrder<T, offset, Sz>::get(pbuf);
    }
  }

  void testSet() {

    T max_packed = PackerGuess<T, Sz>::maxPacked();
    T min_packed = PackerGuess<T, Sz>::minPacked();

    checkSet(0, "zero");
    checkSet(1, "one");

    checkSet(T{1u} << (Sz - 1), "highest bit");
    checkSet(CodeGen<T>::value & max_packed, "Testvalue");

    checkSet(max_packed, "max packed");
    checkSet(max_packed - 1, "max packed minus 1");

    checkSet(min_packed, "min packed");
    checkSet(min_packed + 1, "min packed plus 1");
  }

  void testGet() {

    T max_packed = PackerGuess<T, Sz>::maxPacked();
    T min_packed = PackerGuess<T, Sz>::minPacked();

    checkGet(0, "zero");
    checkGet(1, "one");

    checkGet(T{1u} << (Sz - 1), "highest bit");
    checkGet(CodeGen<T>::value & max_packed, "Testvalue");

    checkGet(max_packed, "max packed");
    checkGet(max_packed - 1, "max packed minus 1");

    checkGet(min_packed, "min packed");
    checkGet(min_packed + 1, "min packed plus 1");
  }
};


TYPED_TEST_SUITE_P(LEOrderTestSuite);
TYPED_TEST_P(LEOrderTestSuite, TestSet) {
  this->testSet();
}

TYPED_TEST_P(LEOrderTestSuite, TestGet) {
  this->testGet();
}

REGISTER_TYPED_TEST_SUITE_P
(
    LEOrderTestSuite,
    TestSet,
    TestGet
);

INSTANTIATE_TYPED_TEST_SUITE_P
(
    TestLEOrder,
    LEOrderTestSuite,
    AllTestArgs,
);

}  // [anonymous]
