///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
///
///
///
namespace test_helpers{

template <typename T, uint16_t Sz>
struct Helper {
  typedef typename std::conditional<
    std::is_integral<T>::value && std::is_signed<T>::value,
      typename std::make_unsigned<T>::type, T>::type packedT;

  static T maxPacked() {
    T x = std::numeric_limits<T>::max();
    for(uint16_t i=Sz; i < sizeof(T) * 8; i++) {
      x >>= 1;
    }
    return x;
  }
  static T minPacked() {
    T x = std::numeric_limits<T>::min();
    for(uint16_t i=Sz; i < sizeof(T) * 8; i++) {
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
  static packedT expectedPack(T value)
  {
    if(std::is_floating_point<T>::value) {
      return static_cast<packedT>(value);
    } else if(std::is_signed<T>::value) {
      packedT x = static_cast<packedT>(value);
      if(value > maxPacked()) {
        x = static_cast<packedT>(maxPacked());
      } else if (value < minPacked()) {
        x = static_cast<packedT>(minPacked());
      }
      packedT mask = 0;
      for(int i =0; i < Sz; i++)
      {
        mask <<= 1;
        mask |= 1;
      }
      return x & mask;
    } else {
      packedT x = static_cast<packedT>(value);
      if(value > maxPacked()) {
        x = static_cast<packedT>(maxPacked());
      }
      packedT mask = 0;
      for(int i =0; i < Sz; i++)
      {
        mask <<= 1;
        mask |= 1;
      }
      return x & mask;
    }
  }
  static T expectedUnpack(packedT value)
  {
    if(std::is_floating_point<T>::value) {
      return static_cast<T>(value);
    } else if(std::is_signed<T>::value) {
      packedT x = value;
      packedT mask = 0;
      for(int i =0; i < (Sz - 1); i++)
      {
        mask <<= 1;
        mask |= 1;
      }
      packedT neg_mask = ~mask;
      x &= mask;
      packedT signed_bit = 1u;
      for(int i =0; i < (Sz - 1); i++)
      {
        signed_bit <<= 1;
      }

      if (value & signed_bit) {
        x |= neg_mask;
      }
      return static_cast<T>(x);
    } else {
      packedT x = value;
      packedT mask = 0;
      for(int i =0; i < Sz; i++)
      {
        mask <<= 1;
        mask |= 1;
      }
      return x & mask;
    }
  }
};



}  // namespace test_helpers
