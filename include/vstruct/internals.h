///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
/// This file provides the internal mechanisms used to access data.
///
///
#ifndef XVSTRUCT_INTERNALS_H_
#define XVSTRUCT_INTERNALS_H_

#include <assert.h>
#include <stdint.h>
#include <limits>
#include <type_traits>
#include <assert.h>

namespace vstruct {

typedef uint8_t pbuf_type;

namespace internals {

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Templated iteration to get Max value
////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, uint16_t Sz>  // add one bit for each bit in size
struct MaskMax {
  enum : T {
    value = std::is_signed<T>::value ?
      MaskMax<typename std::make_unsigned<T>::type, Sz - 1>::value :  // one bit is used for sign
      (MaskMax<T, Sz - 1>::value << 1) | 1  // add one bit
  };
};

template <typename T>  // final iteration
struct MaskMax<T, 0> {
  enum : T {
    value = 0,
  };
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Templated iteration for Little Endian
/// Iteration starts from forArg ... 1; 0 is special case to stop
////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T, uint16_t forArg, uint16_t offset>  // loop down
struct LEForLoop {
  static T get(pbuf_type* pData, T x) {
    x |= (pData[forArg]) << ((forArg * 8) - offset);
    return LEForLoop<T, forArg - 1, offset>::get(pData, x);
  }
  static void set(uint8_t * pData, T x) {
    pData[forArg] = x >> ((forArg * 8) - offset);
    LEForLoop<T, forArg - 1, offset>::set(pData, x);
  }
};

template <typename T, uint16_t offset>  // final iteration
struct LEForLoop<T, 0, offset> {
  static T get(pbuf_type* pData, T x) { return x | static_cast<T>(pData[0] >> offset); }
  static void set(uint8_t * pData, T x) { }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Byte manipulation
////////////////////////////////////////////////////////////////////////////////////////////////////////

template <uint16_t offset, uint16_t Sz>
struct ByteManipulator {
  static_assert(offset + Sz <= 8, "unexpected value of offset + Sz, need to fit in a byte");
  static void set(pbuf_type* pData, pbuf_type value) {

  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class definitions
////////////////////////////////////////////////////////////////////////////////////////////////////////

/// TypeBase - all type objects should derive from this
template <typename T, uint16_t param_bits, uint16_t param_Sz>
struct TypeBase {
  typedef T unpackedT;
  typedef typename std::conditional<std::is_signed<T>::value && !std::is_floating_point<T>::value,
                                    typename std::make_unsigned<T>::type, T>::type packedT;
  enum : uint16_t {
    bits = param_bits,
    Sz = param_Sz,
    B = bits >> 3,
    b = bits & 0x7u
  };
  uint16_t firstByte() {
    return bits >> 3;
  }
  uint16_t firstBit() {
    return bits;
  }
  uint16_t elementByteSize() {  // byte size of this item or list, rounded up
    return Sz + 7 >> 3;
  }
  uint16_t elementBitSize() {  // bit size of this item or list
    return Sz;
  }
  uint16_t nextBit(){  // position of bit for next item or list
    return bits + Sz;
  }
  uint16_t previousByteSize(){ // total bytes up to previous (excluding this)
    return (bits + 7) >> 3 ;
  }
  uint16_t cummulativeByteSize(){  // total bytes up to this (including this)
    return (bits + Sz + 7) >> 3 ;
  }
protected:
  ~TypeBase(){}
};

/// LEOrder - methods to get/set in little endian order
template <class T, uint16_t offset, uint16_t Sz>
struct LEOrder {
  static_assert(Sz >= 1, "0 sized Item is not supported");
  static_assert(
      std::is_integral<T>::value && std::is_unsigned<T>::value && !std::is_same<T, bool>::value,
      "This interface class can only deal with unsigned integer types");
  enum : uint16_t {
    last_byte = ((offset + Sz - 1) >> 3),
    last_byte_minus_1 = last_byte > 0 ? last_byte - 1 : 0
  };
  enum : T {
    mask = MaskMax<T, Sz>::value
  };
  // pData is the pointer to the FIRST data position (not start of buffer)
  // Get Bits from buffer, Little Endian Ordering
  static T get(uint8_t* pData) {
    T x = 0;
    x = LEForLoop<T, last_byte, offset>::get(pData, x);
    x &= mask;
    volatile T m = mask;
    return x;
  }

  template<bool Valid=true>
  static void setLastByte(pbuf_type* pData, T x) {
    if ((Sz + offset) & 7) {  // if last bit is not aligned to 8, must use mask
      *pData &= ~(mask >> (last_byte * 8 - offset));
      *pData |= x >> (last_byte * 8 - offset);
    } else {  // bits are aligned to end of byte, can just copy
      *pData = x >> (last_byte * 8 - offset);
    }
  }

  // Write Bits from buffer, Little Endian Ordering
  static void set(pbuf_type* pData, T x) {
    static_assert(Sz >= 1, "0 sized object");

    x &= mask;
    // first byte
    if (Sz < 8 || offset > 0) {  // if first bit does not fully fill byte or is not aligned, must use bit mask
      pData[0] &= ~(mask << offset);
      pData[0] |= (x << offset);
    } else {  // direct copy
      pData[0] = x;
    }

    if (last_byte > 0) {
      // middle bytes
      LEForLoop<T, last_byte_minus_1, offset>::set(pData, x);
      // last byte
      // setLastByte<(last_byte > 0)>(&pData[last_byte], x);

      if ((Sz + offset) & 7) {  // if last bit is not aligned to 8, must use mask
        *pData &= ~(mask >> (last_byte_minus_1 * 8 + offset));
        *pData |= x >> (last_byte_minus_1 * 8 + offset);
      } else {  // bits are aligned to end of byte, can just copy
        *pData = x >> (last_byte_minus_1 * 8 + offset);
      }
    }
  }
};

/// Packer - pack value including sign bits
template <typename T, uint16_t Sz>
struct Packer {
  typedef typename std::conditional<std::is_signed<T>::value && !std::is_floating_point<T>::value,
                                    typename std::make_unsigned<T>::type, T>::type packedT;
  // pack
  static packedT pack(T x) {
    packedT max_val = static_cast<packedT>(MaskMax<T, Sz>::value);
    packedT min_val = ~max_val;
    packedT mask = MaskMax<packedT, Sz>::value;

    if (std::is_floating_point<T>::value) {
      // dont touch the floating point
    } else if (x > static_cast<T>(max_val)) {
      x = max_val;
    } else if (std::is_signed<T>::value && (x < static_cast<T>(min_val))) {
      x = min_val;
    }
    return static_cast<packedT>(x) & mask;
  }

  // unpack
  static T unpack(packedT x) {
    packedT max_val = static_cast<packedT>(MaskMax<T, Sz>::value);
    packedT min_val = ~max_val;
    packedT mask = MaskMax<packedT, Sz>::value;
    x &= mask;
    if (std::is_floating_point<T>::value) {
      // dont touch the floating point
    } else if (std::is_signed<T>::value && !std::is_floating_point<T>::value && (min_val & x)) {
      x |= min_val;
    }
    return static_cast<T>(x);
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Packer Specialization
////////////////////////////////////////////////////////////////////////////////////////////////////////

template <uint16_t Sz>  // specialization for float type
struct Packer<float, Sz> {
  static_assert(Sz == 32, "Size must be 32bits for float type");
  typedef uint32_t packedT;
  static float unpack(packedT x) {
    packedT* p = &x;
    return *(reinterpret_cast<float*>(p));
  }
  static packedT pack(float x) {
    float* p = &x;
    return *(reinterpret_cast<packedT*>(p));
  }
};


template <uint16_t Sz>  // specialization for double type
struct Packer<double, Sz> {
  static_assert(Sz == 64, "Size must be 64bits for double type");
  typedef uint64_t packedT;
  static double unpack(packedT x) {
    packedT* p = &x;
    return *(reinterpret_cast<double*>(p));
  }
  static packedT pack(double x) {
    double* p = &x;
    return *(reinterpret_cast<packedT*>(p));
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Temporary Objects
////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Temporary object created when Array index is accessed.
/// assert by default. code generaotr should populate this.
template<typename T, uint16_t Sz, uint16_t N>
struct LEArrayTemp {
  pbuf_type* &pData_;
  const uint16_t index_;

  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  LEArrayTemp(pbuf_type* &pData, uint16_t index): pData_(pData), index_(index) {
    assert(index < N);
  }

  // getter and setter
  operator T () const {
    assert(false && "indexer must be specialized first!");
  }

  LEArrayTemp<T, Sz, N>& operator= (const T& value) {
    assert(false && "indexer must be specialized first!");
  }
};


/// Temporary object created when BoolArray index is accessed.
template<uint16_t b, uint16_t N>
struct BoolArrayTemp {
  pbuf_type* &pData_;
  const uint16_t index_;

  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  BoolArrayTemp(pbuf_type* &pbuf, uint16_t index):
    pData_(pbuf),
    index_(index) {
    assert(index < N && "Index is out of bounds!");
  }
  operator bool () const {
    uint16_t B_ = (b + index_) >> 3;
    uint16_t b_ = (b + index_) & 0x7;
    return static_cast<bool>(pData_[B_] & (1u << b_));
  }

  BoolArrayTemp& operator= (const bool& value) {
    uint16_t B_ = (b + index_) >> 3;
    uint16_t b_ = (b + index_) & 0x7;
    if (value) {
      pData_[B_] |= (1u << b_);
    } else {
      pData_[B_] &= ~(1u << b_);
    }
  }
};



}  // namespace internals
}  // namespace xvstruct



#endif  // INCLUDE_XVSTRUCT_INTERNALS_H_

