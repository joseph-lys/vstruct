///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
///
///
///
#ifndef TEST_TESTLIB_H_
#define TEST_TESTLIB_H_

#include <cstdlib>
#include <limits>
#include <type_traits>

namespace test_helpers {

template <typename T, u_int16_t Sz>
struct PackerGuessHelperCommon {
  static T maxPacked() {
    T x = std::numeric_limits<T>::max();
    for (uint16_t i = Sz; i < sizeof(T) * 8; i++) {
      x >>= 1;
    }
    return x;
  }
  static T minPacked() {
    T x = std::numeric_limits<T>::min();
    for (uint16_t i = Sz; i < sizeof(T) * 8; i++) {
      x >>= 1;
    }
    return x;
  }
  static T maxUnpacked() {
    return std::numeric_limits<T>::max();
  }
  static T minUnpacked() {
    return std::numeric_limits<T>::min();
  }
};

template <typename T, u_int16_t Sz, bool isfloat, bool issigned>
struct PackerGuessHelper;

template <typename T, u_int16_t Sz, bool issigned>
struct PackerGuessHelper<T, Sz, true, issigned> : public PackerGuessHelperCommon<T, Sz> {
  static T expected(T value) {
    return value;
  }
};

template <typename T, u_int16_t Sz>
struct PackerGuessHelper<T, Sz, false, true> : public PackerGuessHelperCommon<T, Sz> {
  typedef typename std::make_unsigned<T>::type packedT;
  static packedT expectedPack(T value) {
    T max_packed = PackerGuessHelper::maxPacked();
    T min_packed = PackerGuessHelper::minPacked();
    if (value > max_packed) {
      value = max_packed;
    }
    if (value < min_packed) {
      value = min_packed;
    }
    packedT bit_mask = (static_cast<packedT>(max_packed) << 1) | 1;  // add the missing bit used for sign.
    return static_cast<packedT>(value) & bit_mask;
  }
  static T expectedUnpack(packedT value) {
    packedT pos_mask = static_cast<packedT>(PackerGuessHelper::maxPacked());
    packedT neg_mask = ~pos_mask;
    if (neg_mask & value) {
      value |= neg_mask;
    } else {
      value &= pos_mask;
    }
    return static_cast<T>(value);
  }
  static T expected(T value) {
    return expectedUnpack(expectedPack(value));
  }
};

template <typename T, u_int16_t Sz>
struct PackerGuessHelper<T, Sz, false, false> : public PackerGuessHelperCommon<T, Sz> {
  typedef T packedT;
  static packedT expectedPack(T value) {
    packedT pos_mask = PackerGuessHelper::maxPacked();
    if (value > pos_mask) {
      value = pos_mask;
    }
    return value;
  }
  static T expectedUnpack(packedT value) {
    packedT pos_mask = PackerGuessHelper::maxPacked();
    return value & pos_mask;
  }
  static T expected(T value) {
    return expectedUnpack(expectedPack(value));
  }
};

template <typename T, uint16_t Sz>
struct PackerGuess : PackerGuessHelper<T, Sz, std::is_floating_point<T>::value, std::is_signed<T>::value> {};

template <typename T, size_t s> struct CodeGenHelper;
template <typename T>
struct CodeGenHelper<T, 1> {
  enum : T {
    value = T(0x1)
  };
};
template <typename T>
struct CodeGenHelper<T, 2> {
  enum : T {
    value = T(0x21)
  };
};
template <typename T>
struct CodeGenHelper<T, 4> {
  enum : T {
    value = T(0x8421)
  };
};
template <typename T>
struct CodeGenHelper<T, 8> {
  enum : T {
    value = T(0x842148421)
  };
};
template <typename T>
struct CodeGen : public CodeGenHelper<T, sizeof(T)> {};


static unsigned int rand_seed = 12345;  // use a fix random seed
template <typename T, bool is_float> struct RandomValueHelper;

// random value for floating point types
template <typename T> struct RandomValueHelper<T, true> {
  T randomValue(){
    T x = static_cast<T>(rand_r(&rand_seed)) / static_cast<T>(RAND_MAX);
    if (rand_r(&rand_seed) < (RAND_MAX / 2 )) {  // randomly make negative
      x = -x;
    }
    x *= std::numeric_limits<T>::max();
    return x;
  }
};
// random value for non-floating point types
template <typename T> struct RandomValueHelper<T, false> {
  // will never hit high primes, but should not be relevant for our tests
  T randomValue(){
    T x = 1;
    for (int i = 0; i < sizeof(T); i += sizeof(decltype(rand_r(&rand_seed)))) {
      x *= rand_r(&rand_seed);
    }
    if (std::is_signed<T>::value && rand_r(&rand_seed) < (RAND_MAX / 2 )) {  // randomly make negative
      x = -x;
    }
    return x;
  }
};


template <typename T> struct RandomValue : public RandomValueHelper<T, std::is_floating_point<T>::value> {};



}  // namespace test_helpers


#endif  // TEST_TESTLIB_H_
