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
#include "../testlib.h"

namespace {

template <typename TypeArg, uint16_t SzArg>
struct TestArgs {
  typedef TypeArg T;
  enum : uint16_t {
    Sz = SzArg
  };
};

using testing::Types;
typedef Types<
TestArgs<uint8_t, 2>,
TestArgs<uint8_t, 7>,
TestArgs<uint8_t, 8>,
TestArgs<uint16_t, 3>,
TestArgs<uint16_t, 15>,
TestArgs<uint16_t, 16>,
TestArgs<uint32_t, 4>,
TestArgs<uint32_t, 23>,
TestArgs<uint32_t, 24>,
TestArgs<uint32_t, 25>,
TestArgs<uint32_t, 31>,
TestArgs<uint32_t, 32>,
TestArgs<uint64_t, 5>,
TestArgs<uint64_t, 11>,
TestArgs<uint64_t, 17>,
TestArgs<uint64_t, 29>,
TestArgs<uint64_t, 63>,
TestArgs<uint64_t, 64>
> UnsignedTestArgs;

using testing::Types;
typedef Types<
TestArgs<int8_t, 2>,
TestArgs<int8_t, 7>,
TestArgs<int8_t, 8>,
TestArgs<int16_t, 3>,
TestArgs<int16_t, 15>,
TestArgs<int16_t, 16>,
TestArgs<int32_t, 4>,
TestArgs<int32_t, 23>,
TestArgs<int32_t, 24>,
TestArgs<int32_t, 25>,
TestArgs<int32_t, 31>,
TestArgs<int32_t, 32>,
TestArgs<int64_t, 5>,
TestArgs<int64_t, 11>,
TestArgs<int64_t, 17>,
TestArgs<int64_t, 29>,
TestArgs<int64_t, 63>,
TestArgs<int64_t, 64>
> SignedTestArgs;



using test_helpers::PackerGuess;
template <typename TArgs>
class PackerTestSuite : public testing::Test {
 public:
  typedef typename TArgs::T T;

  typedef typename std::conditional<
    std::is_integral<T>::value && std::is_signed<T>::value,
      typename std::make_unsigned<T>::type, T>::type packedT;
  enum : uint16_t {
    Sz = TArgs::Sz
  };
  void checkPack(T value, const char debug_str[]) {
    packedT output = vstruct::internals::Packer<T, Sz>::pack(value);
    packedT expected = PackerGuess<T, Sz>::expectedPack(value);
    EXPECT_EQ(output, expected)
        << debug_str
        << ", value:" << static_cast<int64_t>(value) << " Sz:" << Sz
        << " max:" << static_cast<int64_t>(PackerGuess<T, Sz>::maxPacked())
        << " min:" << static_cast<int64_t>(PackerGuess<T, Sz>::minPacked());
    if (output != expected) {
      volatile packedT debugging = vstruct::internals::Packer<T, Sz>::pack(value);
    }
  }

  void checkUnpack(T value, const char debug_str[]) {
    packedT pvalue = test_helpers::PackerGuess<T, Sz>::expectedPack(value);
    T output = vstruct::internals::Packer<T, Sz>::unpack(pvalue);
    T expected = PackerGuess<T, Sz>::expectedUnpack(pvalue);
    EXPECT_EQ(output, expected)
        << debug_str
        << ", value:" << static_cast<int64_t>(value) << " Sz:" << Sz
        << " max:" << static_cast<int64_t>(PackerGuess<T, Sz>::maxPacked())
        << " min:" << static_cast<int64_t>(PackerGuess<T, Sz>::minPacked());
    if (output != expected) {
      volatile packedT debugging = vstruct::internals::Packer<T, Sz>::pack(value);
    }
  }

  void testPack() {
    T max_packed = PackerGuess<T, Sz>::maxPacked();
    T min_packed = PackerGuess<T, Sz>::minPacked();
    T max_unpacked = PackerGuess<T, Sz>::maxUnpacked();
    T min_unpacked = PackerGuess<T, Sz>::minUnpacked();

    checkPack(0, "zero");
    checkPack(-1, "minus 1");
    checkPack(1, "minus 1");

    checkPack(max_packed, "max packed");
    checkPack(max_packed - 1, "max packed minus 1");
    checkPack(max_packed + 1, "max packed plus 1");

    checkPack(min_packed, "min packed");
    checkPack(min_packed - 1, "min packed minus 1");
    checkPack(min_packed + 1, "min packed plus 1");

    checkPack(max_unpacked, "max unpacked");
    checkPack(max_unpacked - 1, "max unpacked minus 1");
    checkPack(max_unpacked + 1, "max unpacked plus 1");

    checkPack(min_unpacked, "min unpacked");
    checkPack(min_unpacked - 1, "min unpacked minus 1");
    checkPack(min_unpacked + 1, "min packed plus 1");
  }

  void testUnpack() {
    T max_packed = PackerGuess<T, Sz>::maxPacked();
    T min_packed = PackerGuess<T, Sz>::minPacked();
    T max_unpacked = PackerGuess<T, Sz>::maxUnpacked();
    T min_unpacked = PackerGuess<T, Sz>::minUnpacked();

    checkUnpack(0, "zero");
    checkUnpack(-1, "minus 1");
    checkUnpack(1, "minus 1");

    checkUnpack(max_packed, "max packed");
    checkUnpack(max_packed - 1, "max packed minus 1");
    checkUnpack(max_packed + 1, "max packed plus 1");

    checkUnpack(min_packed, "min packed");
    checkUnpack(min_packed - 1, "min packed minus 1");
    checkUnpack(min_packed + 1, "min packed plus 1");

    checkUnpack(max_unpacked, "max unpacked");
    checkUnpack(max_unpacked - 1, "max unpacked minus 1");
    checkUnpack(max_unpacked + 1, "max unpacked plus 1");

    checkUnpack(min_unpacked, "min unpacked");
    checkUnpack(min_unpacked - 1, "min unpacked minus 1");
    checkUnpack(min_unpacked + 1, "min packed plus 1");
  }
};


TYPED_TEST_SUITE_P(PackerTestSuite);
TYPED_TEST_P(PackerTestSuite, TestPack) {
  this->testPack();
}

TYPED_TEST_P(PackerTestSuite, TestUnpack) {
  this->testUnpack();
}

REGISTER_TYPED_TEST_SUITE_P
(
    PackerTestSuite,
    TestPack,
    TestUnpack
);

INSTANTIATE_TYPED_TEST_SUITE_P
(
    TestPackerUnsigned,
    PackerTestSuite,
    UnsignedTestArgs,
);

INSTANTIATE_TYPED_TEST_SUITE_P
(
    TestPackerSigned,
    PackerTestSuite,
    SignedTestArgs,
);

}  // namespace
