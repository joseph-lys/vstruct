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
template <typename T, size_t Sz>  // add one bit for each bit in size
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
template <class T, size_t Sz>
struct LEOrder {
  static_assert(Sz >= 1, "0 sized Item is not supported");
  static_assert(
      std::is_integral<T>::value && std::is_unsigned<T>::value && !std::is_same<T, bool>::value,
      "This interface class can only deal with unsigned integer types");
  enum : size_t {
    nbytes = sizeof(T),
    nbits = nbytes << 3
  };
  enum : T {
    mask = MaskMax<T, Sz>::value
  };
  // pRoot is the pointer to the start of buffer
  // Get Bits from buffer, Little Endian Ordering
  static T get(uint8_t* pRoot, size_t starting_bit) {
    T x;
    T y;
    size_t offset_byte = starting_bit >> 3;
    size_t offset_bit = starting_bit & 0x7;
    x = 0;
    for (size_t i=0; i<nbytes; i++)
    {
      x |= static_cast<T>(pRoot[offset_byte + i]) << (i << 3);
    }
    x >>= offset_bit;
    if (offset_bit + Sz > nbits)
    {
      y = static_cast<T>(pRoot[offset_byte + nbytes]);
      x |= y << (((nbytes) << 3) - offset_bit);
    }
    x &= mask;
    return x;
  }

  // Write Bits from buffer, Little Endian Ordering
  static void set(pbuf_type* pData, size_t starting_bit, T x) {
    static_assert(Sz >= 1, "0 sized object");

    size_t offset_byte = starting_bit >> 3;
    size_t offset_bit = starting_bit & 0x7;
    size_t total_bytes = (offset_bit + Sz + 7) >> 3;
    size_t shift;
    pbuf_type byte_mask;
    x &= mask;
    // first byte
    byte_mask = static_cast<pbuf_type>(mask);
    byte_mask <<= offset_bit;
    pData[offset_byte] &= ~byte_mask;
    pData[offset_byte] |= static_cast<pbuf_type>(x << offset_bit);
    // middle bytes
    if (total_bytes > 2) {  // next byte is a direct copy
      for (size_t i=1; i<total_bytes - 1; i++) {
        pData[offset_byte + i] = x >> ((i << 3) - offset_bit);
      }
    }
    // last byte
    if (total_bytes > 1) {
      shift = ((total_bytes - 1) << 3) - offset_bit;
      byte_mask = static_cast<pbuf_type>(mask >> shift);
      pData[offset_byte + total_bytes - 1] &= ~byte_mask;
      pData[offset_byte + total_bytes - 1] |= static_cast<pbuf_type>(x >> shift);
    }
    
  }
};

/// Packer - pack value including sign bits
template <typename T, size_t Sz>
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
template <size_t param_bits, size_t param_Sz, size_t param_N>
struct TypeBaseFunctions {
  enum : size_t {
    bits = param_bits,  // first bit position
    Sz = param_Sz,
    N = param_N,
    B = bits >> 3,  // first Byte position
    b = bits & 0x7u,  // bit offset

    first_byte = B,
    first_bit = bits,
    byte_size = (Sz * N + 7) >> 3,
    bit_size = Sz * N,
    next_bit = b + (Sz * N),  // for next Item
    prev_bytes = (bits + 7) >> 3,  // total bytes up to previous (excluding this)
    total_bytes = (bits + (Sz * N)  + 7) >> 3  // total bytes up to this (including this)
  };
 protected:
  ~TypeBaseFunctions(){}
};

template <typename T, size_t bits, size_t Sz, size_t N>
struct TypeBase : public TypeBaseFunctions<bits, Sz, N>{
  typedef T unpackedT;
  typedef typename Packer<T, Sz>::packedT packedT;
 protected:
  ~TypeBase(){}
};

// specialization for bool
template <size_t bits, size_t Sz, size_t N>
struct TypeBase <bool, bits, Sz, N> : public TypeBaseFunctions<bits, Sz, N>{
 protected:
  ~TypeBase(){}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Packer Specialization
////////////////////////////////////////////////////////////////////////////////////////////////////////

template <size_t Sz>  // specialization for float type
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


template <size_t Sz>  // specialization for double type
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
template<typename T, size_t Sz>
struct LEArrayTemp {
  pbuf_type* pData_;
  const size_t first_bit_;
  using Packer_ = Packer<T, Sz>;
  typedef typename Packer_::packedT packedT;
  using LEOrder_ = LEOrder<packedT, Sz>;

  LEArrayTemp(pbuf_type* pData, size_t first_bit)
  : pData_(pData), first_bit_(first_bit) {
  }

  // getter
  operator T () const {
    return Packer_::unpack(LEOrder_::get(pData_, first_bit_));
  }

  // setter
  LEArrayTemp<T, Sz>& operator= (const T& value) {
    LEOrder_::set(pData_, first_bit_, Packer_::pack(value));
  }
};


/// Temporary object created when BoolArray index is accessed.
template<size_t offset, size_t N>
struct BoolArrayTemp {
  pbuf_type* const pData_;
  const size_t index_;

  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  BoolArrayTemp(pbuf_type* pbuf, size_t index):
    pData_(pbuf),
    index_(index) {
    assert(index < N && "Index is out of bounds!");
  }
  operator bool () const {
    size_t B_ = (offset + index_) >> 3;
    size_t b_ = (offset + index_) & 0x7;
    return static_cast<bool>(pData_[B_] & (1u << b_));
  }

  BoolArrayTemp& operator= (const bool& value) {
    size_t B_ = (offset + index_) >> 3;
    size_t b_ = (offset + index_) & 0x7;
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

