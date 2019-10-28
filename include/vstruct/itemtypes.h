// todo: some copyright here
//
// xvstruct.h
//
//
// todo: description here
//

#ifndef VSTRUCT_ITEMTYPES_H_
#define VSTRUCT_ITEMTYPES_H_


#include <stdint.h>
#include "./internals.h"


namespace vstruct {
////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Struct declarations and access functions
/// Template args:
///   T: Storage Type
///   bits: first bit position
///   Sz: Number of storage bits
////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint16_t bits, uint16_t Sz>
struct LEItem;  // storage type for Little Endian items

template<typename T, uint16_t bits, uint16_t Sz, uint16_t N>
struct LEArray;  // storage type for Little Endian Arrays

template<uint16_t bits>
struct BoolItem;  // storage type for single bool

template<uint16_t bits, uint16_t N>
struct BoolArray;  // storage type for bool Arrays

template<uint16_t bits, uint16_t AlignBit>
struct AlignPad;  // dummy storage for byte alignment


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Little Endian Integer / Float
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T, uint16_t bits, uint16_t Sz>
struct LEItem final : public internals::TypeBase<T, bits, Sz, 1> {
  static_assert(!std::is_base_of<bool, T>::value, "bool type is not allowed, use BoolItem instead");
  static_assert(!std::is_floating_point<T>::value ||(std::is_floating_point<T>::value && (Sz == (sizeof(T) << 3))),
                "No compression allowed for floating point types, Sz must match floating point sizeof");
  static_assert(Sz > 0, "Size must be 1 or more");
  static_assert(Sz <= 64, "Maximum 64bit _ItemBase supported");
  static_assert(Sz <= 8 * sizeof(T), "Bit packed _ItemBase should be equal or less than Raw _ItemBase");

  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit LEItem(pbuf_type* &pbuf): pbuf_(pbuf) {}

  operator T() const {  // getter
      return internals::Packer<T, Sz>::unpack(
               internals::LEOrder<typename LEItem::packedT, LEItem::b, Sz>::get(&pbuf_[LEItem::B]));
  }

  LEItem<T, LEItem::b, Sz>& operator= (const T& value) {  // setter
      internals::LEOrder<typename LEItem::packedT, LEItem::b, Sz>::set(
        &pbuf_[LEItem::B], internals::Packer<T, Sz>::pack(value));
  }
};

template<typename T, uint16_t bits, uint16_t Sz, uint16_t N>
struct LEArray final : public internals::TypeBase<T, bits, Sz, N> {
  static_assert(!std::is_base_of<T, bool>::value, "bool type is not allowed");
  static_assert(Sz > 0, "Size must be 1 or more");
  static_assert(Sz <= 64, "Maximum 64bit supported");
  static_assert(Sz <= 8 * sizeof(T), "Bit packed should be equal or less than original type");

  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit LEArray(pbuf_type* &pbuf): pbuf_(pbuf) {}

  // index operator is exposed. returns the temporary array object
  internals::LEArrayTemp<T, LEArray::b, Sz, N> operator[](uint16_t index) {
    return internals::LEArrayTemp<T, LEArray::b, Sz, N>{&pbuf_[LEArray::B], index};
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Bool Types
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<uint16_t bits>
struct BoolItem final : public internals::TypeBase<bool, bits, 1, 1> {
  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit BoolItem(pbuf_type* &pbuf): pbuf_(pbuf) {}

  operator bool () const {
    return vstruct::internals::ByteAccess<1, BoolItem::b>::get(&pbuf_[BoolItem::B]);
  }

  BoolItem<BoolItem::b>& operator= (const bool& value) {
    vstruct::internals::ByteAccess<1, BoolItem::b>::set(&pbuf_[BoolItem::B], static_cast<uint8_t>(value));
  }
};

template<uint16_t bits, uint16_t N>
struct BoolArray final : public internals::TypeBase<bool, bits, 1, N> {
  static_assert(N > 0, "Size must be 1 or more");
  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit BoolArray(pbuf_type* &pbuf): pbuf_(pbuf) {}

  internals::BoolArrayTemp<BoolArray::b, N> operator[](uint16_t index) {
    return internals::BoolArrayTemp<BoolArray::b, N>{ &pbuf_[BoolArray::B], index};
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Padding for alignment
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<uint16_t bits, uint16_t AlignByte>
struct AlignPad final {
  static_assert(AlignByte <= 8, "Maximum 8 byte allignment allowed");
  enum : uint16_t {
    misalignment = (bits % (AlignByte * 8)),
    next_bit = (misalignment > 0) ? bits + (AlignByte - misalignment): 0
  };
  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit AlignPad(pbuf_type* &pbuf): pbuf_(pbuf) {}
};




}  // namespace vstruct


#endif  // VSTRUCT_ITEMTYPES_H_
