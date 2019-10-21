#ifndef INCLUDE_XVSTRUCT_INTERNALS_H_
#define INCLUDE_XVSTRUCT_INTERNALS_H_


#include <stdint.h>
#include <limits>
#include <type_traits>

namespace xvstruct {

typedef uint8_t pbuf_type;

namespace internals {

// iteration function for generating maximum value
template <typename T, uint16_t Sz>
struct MaskMax {
  enum : T {
    value = std::is_unsigned<T>::value ?
      MaskMax<typename std::make_unsigned<T>::type, Sz - 1>::value :  // one bit is used for sign
      (MaskMax<T, Sz - 1>::value << 1) | 1  // add one bit
  };
};

template <typename T>
struct MaskMax<T, 0> {
  enum : T {
    value = 0,
  };
};


// class that makes the raw interface to buffer
template <class T, uint16_t b, uint16_t Sz>
struct RawIF {
  static_assert(
      std::is_integral<T>::value && std::is_unsigned<T>::value && !std::is_same<T, bool>::value,
      "This interface class can only deal with unsigned integer types");

  // Get Bits from buffer, Little Endian Ordering
  static T getLE(uint8_t* buffer);

  // Write Bits from buffer, Little Endian Ordering
  static void setLE(uint8_t* buffer, T value);
};

template <typename T, uint16_t Sz>
struct Packer {
  typedef typename std::conditional<std::is_signed<T>::value && !std::is_floating_point<T>::value,
                                    typename std::make_unsigned<T>::type, T>::type packedT;
  static T unpackSign(packedT x) {
    packedT max_val = (packedT)MaskMax<T, Sz>::value;
    packedT min_val = ~max_val;
    if (std::is_floating_point<T>::value) {
      // dont touch the floating point
    } else if (std::is_signed<T>::value && !std::is_floating_point<T>::value && (min_val & x)) {
      x |= min_val;
    }
    return (T)x;
  }

  static packedT packSign(T x) {
    packedT max_val = (packedT)MaskMax<T, Sz>::value;
    packedT min_val = ~max_val;

    if (std::is_floating_point<T>::value) {
      // dont touch the floating point
    } else if (x > (T)max_val) {
      x = max_val;
    } else if (std::is_signed<T>::value && (x < (T)min_val)) {
      x = min_val;
    }
    return (packedT)x;
  }
};

template <uint16_t Sz>
struct Packer<float, Sz> {
  static_assert(Sz == 32, "Size must be 32bits for float type");
  typedef uint32_t packedT;
  static float unpackSign(packedT x) {
    packedT* p = &x;
    return *(reinterpret_cast<float*>(p));
  }
  static packedT packSign(float x) {
    float* p = &x;
    return *(reinterpret_cast<packedT*>(p));
  }
};

template <uint16_t Sz>
struct Packer<double, Sz> {
  static_assert(Sz == 64, "Size must be 64bits for double type");
  typedef uint64_t packedT;
  static double unpackSign(packedT x) {
    packedT* p = &x;
    return *(reinterpret_cast<double*>(p));
  }
  static packedT packSign(double x) {
    double* p = &x;
    return *(reinterpret_cast<packedT*>(p));
  }
};


// this iteration is from forArg ... 1; 0 is special case to stop
template <typename T, uint16_t B, uint16_t forArg, uint16_t offset>
struct ForLoop {
  static T LEGet(pbuf_type* buffer, T x) {
    x |= (buffer[B + forArg]) << ((forArg * 8) - offset);
    return ForLoop<T, B, forArg - 1, offset>::LEGet(buffer, x);
  }
  static void LESet(uint8_t * buffer, T x) {
    buffer[B + forArg] = x >> ((forArg * 8) - offset);
    ForLoop<T, B, forArg - 1, offset>::LESet(buffer, x);
  }
};

template <typename T, uint16_t B, uint16_t offset>
struct ForLoop<T, B, 0, offset> {
  static T LEGet(pbuf_type* buffer, T x) { return x; }
  static void LESet(uint8_t * buffer, T x) { }
};


template <typename T, uint16_t pb, uint16_t Sz>
T RawIF<T, pb, Sz>::getLE(uint8_t* buffer) {
  static_assert(Sz >= 1, "0 sized object");
  T x = 0;
  enum: uint16_t{
    B = pb >> 3,
    b = pb & 7u,
    last_byte = ((b + Sz - 1) >> 3)
  };

  x = buffer[B] >> b;
  x = ForLoop<T, B, last_byte, b>::LEGet(buffer, x);

  return x &= MaskMax<T, Sz>::value;
}

template <typename T, uint16_t pb, uint16_t Sz>
void RawIF<T, pb, Sz>::setLE(pbuf_type* buffer, T x) {
  static_assert(Sz >= 1, "0 sized object");
  T mask = MaskMax<T, Sz>::value;
  x &= mask;
  enum : uint16_t {
    B = pb >> 3,
    b = pb & 7u,
    last_byte = ((b + Sz - 1) >> 3),
    last_byte_minus_1 = last_byte > 0 ? last_byte - 1 : 0
  };
  // first byte
  if (Sz < 8 || b > 0) {  // if first bit does not fully fill byte or is not aligned, must use bit mask
    buffer[B] &= ~(mask << b);
    buffer[B] |= (x << b);
  } else {  // direct copy
    buffer[B] = x;
  }

  if (last_byte > 0) {
    // middle bytes
    ForLoop<T, B, last_byte_minus_1, b>::LESet(buffer, x);

    // last byte
    if ((Sz + b) & 7) {  // if last bit is not aligned to 8, must use mask
      buffer[B + last_byte] &= ~(mask >> ((last_byte << 3) - b));
      buffer[B + last_byte] |= x >> ((last_byte << 3) - b);
    } else {  // bits are aligned to end of byte, can just copy
      buffer[B + last_byte] = x >> (last_byte << 3);
    }
  }
}

// helper function to find next matching bit alignment
template<uint16_t b, uint16_t AlignBit>
struct nextAlignedBit {
  enum : uint16_t{
    value = b + (AlignBit - (b % AlignBit))
  };
};

//
/* route according to last set lookup_ variable */
template<typename T, uint16_t b, uint16_t Sz, uint16_t I>
struct IndexedLE {
  enum :uint16_t {
    bi = b + (I * Sz)
  };

  static T getLE(pbuf_type* pbuf, int index) {
    switch (index) {
    case I:
      return RawIF<T, bi, Sz>::getLE(pbuf); break;
    default:
      return IndexedLE<T, b, Sz, I - 1>::getLE(pbuf, index);
    }
  }
  static void setLE(pbuf_type* pbuf, int index, T value) {
    switch (index) {
    case I:
      return RawIF<T, bi, Sz>::setLE(pbuf, value);  break;
    default:
      return IndexedLE<T, b, Sz, I - 1>::setLE(pbuf, index, value);
    }
  }
};

template<typename T, uint16_t b, uint16_t Sz>
struct IndexedLE <T, b, Sz, 0> {
  static T getLE(pbuf_type* pbuf, int index) {
    switch (index) {
    case 0:
      return RawIF<T, b, Sz>::getLE(pbuf);
    default:
      return 0;  // Todo: no error or an assert?
    }
  }
  static void setLE(pbuf_type* pbuf, int index, T value) {
    switch (index) {
    case 0:
      return RawIF<T, b, Sz>::setLE(pbuf, value);
    default:
      return;  // Todo: no error or an assert?
    }
  }
};

// Temporary Object used by array
template<typename T, uint16_t b, uint16_t Sz, uint16_t N>
struct LEArrayTemp {
  enum : uint16_t { N_minus_one = N - 1 };
  typedef typename internals::Packer<T, Sz>::packedT packedT;
  pbuf_type* &pbuf_;
  const uint16_t index_;

  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  LEArrayTemp(pbuf_type* &pbuf, uint16_t index): pbuf_(pbuf), index_(index) {}

  /* getter and setter */
  operator T () const {
    packedT x = IndexedLE<packedT, b, Sz, N_minus_one>::getLE(pbuf_, index_);
    return Packer<T, Sz>::unpackSign(x);
  }

  LEArrayTemp<T, b, Sz, N>& operator= (const T& value) {
    packedT x = Packer<T, Sz>::packSign(value);
    IndexedLE<packedT, b, Sz, N_minus_one>::setLE(pbuf_, index_, x);
  }
};


struct BoolArrayTemp {
  pbuf_type* &pbuf_;
  const uint16_t B_;
  const uint16_t b_;

  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  BoolArrayTemp(pbuf_type* &pbuf, uint16_t b):
    pbuf_(pbuf),
    B_(b>> 3),
    b_(b & 7u)
    {}
  operator bool () const {
    return static_cast<bool>(pbuf_[B_] & (1u << b_));
  }

  BoolArrayTemp& operator= (const bool& value) {
    if (value) {
      pbuf_[B_] |= (1u << b_);
    } else {
      pbuf_[B_] &= ~(1u << b_);
    }
  }
};

}  // namespace internals


}  // namespace xvstruct





#endif  // INCLUDE_XVSTRUCT_INTERNALS_H_

