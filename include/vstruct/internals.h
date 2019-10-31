///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
/// This file provides the internal mechanisms used to access data.
///
///
#ifndef VSTRUCT_INTERNALS_H_
#define VSTRUCT_INTERNALS_H_

#include <assert.h>
#include <stdint.h>
#include <limits>
#include <type_traits>

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
    x |= static_cast<T>(pData[forArg]) << ((forArg * 8) - offset);
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
/// Byte Access
////////////////////////////////////////////////////////////////////////////////////////////////////////

// access helper
template<size_t Sz, size_t offset>
struct ByteSubStruct {
    union {
        struct {
            uint8_t pad: offset;
            uint8_t element : Sz;
        } bits;
        uint8_t reg;
    };
};

// access helper specializiation for 0 offset
template<size_t Sz>
struct ByteSubStruct<Sz, 0> {
    union {
        struct {
            uint8_t element : Sz;
        } bits;
        uint8_t reg;
    };
};

template<size_t Sz, size_t offset = 0>
struct ByteAccess final{
    static_assert(Sz + offset <= 8, "must fit in a single byte");
    using ByteStruct = ByteSubStruct <Sz, offset>;
    static_assert(sizeof(ByteStruct) == 1, "compiler unexpected uses an invalid size");

    static void set(pbuf_type* buf, uint8_t value) {
        reinterpret_cast<ByteStruct*>(buf)->bits.element = value;
    }
    static uint8_t get(pbuf_type* buf) {
        return reinterpret_cast<ByteStruct*>(buf)->bits.element;
    }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class definitions
////////////////////////////////////////////////////////////////////////////////////////////////////////

/// LEOrder - methods to get/set in little endian order
template <class T, uint16_t offset, uint16_t Sz>
struct LEOrder {
  static_assert(Sz >= 1, "0 sized Item is not supported");
  static_assert(
      std::is_integral<T>::value && std::is_unsigned<T>::value && !std::is_same<T, bool>::value,
      "This interface class can only deal with unsigned integer types");
  enum : uint16_t {
    last_byte = ((offset + Sz - 1) >> 3),
    last_byte_minus_1 = last_byte > 0 ? last_byte - 1 : 0,
    shift_last = last_byte > 0 ? (last_byte * 8 - offset) : 0,  // make it static to remove warnings

    offset_first = offset,  // offset of first byte
    Sz_first = (offset + Sz) > 8 ? (8 - offset) : Sz,  // bits to write on first byte
    offset_last = 0,  // offset of last byte
    Sz_last = ((offset + Sz) % 8) > 0 ? (offset + Sz) % 8 : 8  // bits to write on last byte
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
    return x;
  }

  // Write Bits from buffer, Little Endian Ordering
  static void set(pbuf_type* pData, T x) {
    static_assert(Sz >= 1, "0 sized object");

    x &= mask;
    // first byte
    ByteAccess<Sz_first, offset_first>::set(pData, static_cast<pbuf_type>(x));

    if (last_byte > 0) {
      // middle bytes
      LEForLoop<T, last_byte_minus_1, offset>::set(pData, x);
      // last byte
      ByteAccess<Sz_last, offset_last>::set(&pData[last_byte], static_cast<pbuf_type>(x >> shift_last));
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

/// TypeBase - all type objects should derive from this
template <uint16_t param_bits, uint16_t param_Sz, uint16_t param_N>
struct TypeBaseFunctions {
  enum : uint16_t {
    bits = param_bits,  // first bit position
    Sz = param_Sz,
    N = param_N,
    B = bits >> 3,  // first Byte position
    b = bits & 0x7u,  // bit offset
    next_bit = b + (Sz * N)  // for next Item
  };
  uint16_t firstByte() {
    return B;
  }
  uint16_t firstBit() {
    return bits;
  }
  uint16_t elementByteSize() {  // byte size of this item or list, rounded up
    return (Sz * N) + 7 >> 3;
  }
  uint16_t elementBitSize() {  // bit size of this item or list
    return Sz * N;
  }
  uint16_t nextBit() {  // position of bit for next item or list
    return bits + (Sz * N);
  }
  uint16_t previousByteSize() {  // total bytes up to previous (excluding this)
    return (bits + 7) >> 3;
  }
  uint16_t cummulativeByteSize() {  // total bytes up to this (including this)
    return (bits + (Sz * N)  + 7) >> 3;
  }

 protected:
  ~TypeBaseFunctions(){}
};

template <typename T, uint16_t bits, uint16_t Sz, uint16_t N>
struct TypeBase : public TypeBaseFunctions<bits, Sz, N>{
  typedef T unpackedT;
  typedef typename Packer<T, Sz>::packedT packedT;
 protected:
  ~TypeBase(){}
};

// specialization for bool
template <uint16_t bits, uint16_t Sz, uint16_t N>
struct TypeBase <bool, bits, Sz, N> : public TypeBaseFunctions<bits, Sz, N>{
 protected:
  ~TypeBase(){}
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
/// Templated Binary Search Tree
////////////////////////////////////////////////////////////////////////////////////////////////////////
template <
    class T, uint16_t offset, uint16_t Sz,
    uint16_t Upper, uint16_t Lower,
    bool Valid = Upper >= Lower>
struct LEBinarySearch{
  typedef typename Packer<T, Sz>::packedT packedT;
  // should not be possible to call default.
  static packedT get(pbuf_type* pData, uint16_t index) {
    return packedT(0);
  }
  static void set(pbuf_type* pData, uint16_t index, packedT value) {
    return;
  }
};

template <
    class T, uint16_t offset, uint16_t Sz,
    uint16_t Upper, uint16_t Lower>
struct LEBinarySearch<T, offset, Sz, Upper, Lower, true> {
  enum : uint16_t {
    range = (Upper - Lower) >> 1,
    next_step = (Lower + range) > 0 ? Lower + range - 1 : Lower + range,
    BN = (offset + (Sz * Upper)) >> 3,
    offsetN = (offset + (Sz * Upper)) & 0x7
  };
  using LEOrder_ = LEOrder<T, offsetN, Sz>;
  static T get(pbuf_type* pData, int index) {
    if (Upper == index) {  // actual get operation
      return LEOrder_::get(&pData[BN]);
    } else if (index <= next_step) {  // go to next value in upper block
      return LEBinarySearch<T, offset, Sz, next_step, Lower>::get(pData, index);
    } else {  // go to lower block
      return LEBinarySearch<T, offset, Sz, Upper -1, next_step + 1>::get(pData, index);
    }
  }
  static void set(pbuf_type* pData, int index, T value) {
    if (Upper == index) {  // actual set operation
        LEOrder_::set(&pData[BN], value);
    } else if (index <= next_step) {  // go to next value in upper block
        return LEBinarySearch<T, offset, Sz, next_step, Lower>::set(pData, index, value);
    } else {  // go to lower block
        return LEBinarySearch<T, offset, Sz, Upper -1, next_step + 1>::set(pData, index, value);
    }
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Temporary Objects
////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Temporary object created when Array index is accessed.
/// assert by default. code generaotr should populate this.
template<typename T, uint16_t offset, uint16_t Sz, uint16_t N>
struct LEArrayTemp {
  pbuf_type* pData_;
  const uint16_t index_;
  using Packer_ = Packer<T, Sz>;
  typedef typename Packer_::packedT packedT;
  using LEBinarySearch_ = LEBinarySearch<packedT, offset, Sz, N -1, 0>;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  LEArrayTemp(pbuf_type* pData, uint16_t index): pData_(pData), index_(index) {
    assert(index < N);
  }

  // getter
  operator T () const {
    return Packer_::unpack(LEBinarySearch_::get(pData_, index_));
  }

  // setter
  LEArrayTemp<T, offset, Sz, N>& operator= (const T& value) {
    LEBinarySearch_::set(pData_, index_, Packer_::pack(value));
  }
};


/// Temporary object created when BoolArray index is accessed.
template<uint16_t offset, uint16_t N>
struct BoolArrayTemp {
  pbuf_type* const pData_;
  const uint16_t index_;

  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  BoolArrayTemp(pbuf_type* pbuf, uint16_t index):
    pData_(pbuf),
    index_(index) {
    assert(index < N && "Index is out of bounds!");
  }
  operator bool () const {
    uint16_t B_ = (offset + index_) >> 3;
    uint16_t b_ = (offset + index_) & 0x7;
    return static_cast<bool>(pData_[B_] & (1u << b_));
  }

  BoolArrayTemp& operator= (const bool& value) {
    uint16_t B_ = (offset + index_) >> 3;
    uint16_t b_ = (offset + index_) & 0x7;
    if (value) {
      pData_[B_] |= (1u << b_);
    } else {
      pData_[B_] &= ~(1u << b_);
    }
  }
};



}  // namespace internals
}  // namespace vstruct



#endif  // VSTRUCT_INTERNALS_H_

