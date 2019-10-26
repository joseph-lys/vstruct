// todo: some copyright here
//
// xvstruct.h
//
//
// todo: description here
//

#ifndef INCLUDE_VSTRUCT_ITEMTYPES_H_
#define INCLUDE_VSTRUCT_ITEMTYPES_H_


#include <stdint.h>
#include "./internals.h"


namespace vstruct{
////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Struct declarations and access functions
/// Template args:
///   T: Storage Type
///   b: first bit position
///   Sz: Number of storage bits
////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint16_t b, uint16_t Sz>
struct LEItem;  // storage type for Little Endian items

template<typename T, uint16_t b, uint16_t Sz, uint16_t N>
struct LEArray;  // storage type for Little Endian Arrays

template<uint16_t b>
struct BoolItem;  // storage type for single bool

template<uint16_t b, uint16_t N>
struct BoolArray;  // storage type for bool Arrays

template<uint16_t b, uint16_t AlignBit>
struct AlignPad;  // dummy storage for byte alignment


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Little Endian Integer / Float
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T, uint16_t b, uint16_t Sz>
struct LEItem final : public internals::TypeBase<T, b, Sz>
{
  static_assert(!std::is_base_of<bool, T>::value, "bool type is not allowed, use BoolItem instead");
  static_assert(!std::is_floating_point<T>::value ||(std::is_floating_point<T>::value && (Sz == (sizeof(T) << 3))),
                "No compression allowed for floating point types, Sz must match floating point sizeof");
  static_assert(Sz > 0, "Size must be 1 or more");
  static_assert(Sz <= 64, "Maximum 64bit _ItemBase supported");
  static_assert(Sz <= 8 * sizeof(T), "Bit packed _ItemBase should be equal or less than Raw _ItemBase");

  pbuf_type* &pbuf_;
  LEItem(pbuf_type* &pbuf): pbuf_(pbuf) {}

  operator T() const {  //getter
      return internals::Packer<T, Sz>::unpack(internals::LEOrder<typename LEItem::packedT, b, Sz>::get(&pbuf_[LEItem::B]));
  }

  LEItem<T, b, Sz>& operator= (const T& value) {  //setter
      internals::LEOrder<typename LEItem::packedT, b, Sz>::set(&pbuf_[LEItem::B], internals::Packer<T, Sz>::pack(value));
  }
};

template<typename T, uint16_t b, uint16_t Sz, uint16_t N>
struct LEArray : public internals::TypeBase<T, b, Sz>
{
  static_assert(!std::is_base_of<T, bool>::value, "bool type is not allowed");
  static_assert(Sz > 0, "Size must be 1 or more");
  static_assert(Sz <= 64, "Maximum 64bit supported");
  static_assert(Sz <= 8 * sizeof(T), "Bit packed should be equal or less than original type");

  enum : uint16_t { next_bit = b + (Sz * N) }; // for next Item
  pbuf_type* &pbuf_;
  LEArray(pbuf_type* &pbuf): pbuf_(pbuf) {}

  // index operator is exposed. returns the temporary array object
  internals::LEArrayTemp<T, b, Sz> operator[](uint16_t index) {
    return internals::LEArrayTemp<T, b, Sz>{&pbuf_[LEArray::B], index};
  }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Bool Types
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<uint16_t b>
struct BoolItem final
{
  enum : uint16_t { next_bit = b + 1 }; // for next Item
  pbuf_type* &pbuf_;
  BoolItem(pbuf_type* &pbuf): pbuf_(pbuf) {}

  enum : uint16_t {
    B = b >> 3,
    mask = 1u << (b & 7u)
  };
  operator bool () const {
    return static_cast<bool>(pbuf_[B] & mask);
  }

  BoolItem<b>& operator= (const bool& value) {
    if(value) {
      pbuf_[B] |= mask;
    } else {
      pbuf_[B] &= ~mask;
    }
  }
};

template<uint16_t b, uint16_t N>
struct BoolArray final {
  static_assert(N > 0, "Size must be 1 or more");
  enum : uint16_t { next_bit = b + N }; // for next Item
  pbuf_type* &pbuf_;
  BoolArray(pbuf_type* &pbuf): pbuf_(pbuf) {}

  internals::BoolArrayTemp<b, N> operator[](int index) {
    return internals::BoolArrayTemp<b, N>{&pbuf_[BoolArray::B], b + index};
  }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Padding for alignment
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<uint16_t b, uint16_t AlignByte>
struct AlignPad final
{
  static_assert(AlignByte <= 8, "Maximum 8 byte allignment allowed");
  enum : uint16_t {
    misalignment = (b % (AlignByte * 8)),
    next_bit = (misalignment > 0) ? b + (AlignByte - misalignment): 0
  };
  pbuf_type* &pbuf_;
  AlignPad(pbuf_type* &pbuf): pbuf_(pbuf) {}
};




} // namespace xvstruct



#endif  // INCLUDE_VSTRUCT_TYPES_H_
