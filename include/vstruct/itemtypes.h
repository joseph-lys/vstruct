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
/// Type generator for different types, byte position is inferred from Prev type
/// Template args:
///   Prev: type of the previous object. for first item use Root
///   T: Storage Type
///   Sz: Number of storage bits
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename Prev, typename T, size_t Sz>
struct LEItem;  // type generator for Little Endian items

template<typename Prev, typename T, size_t Sz, size_t N>
struct LEArray;  // type generator for  Little Endian Arrays

template<typename Prev>
struct BoolItem;  // type generator for single bool

template<typename Prev, size_t N>
struct BoolArray;  // type generator for bool Arrays

template<typename Prev, size_t AlignByte>
struct AlignPad;  // type generator for byte alignment

struct Root;  // root item for first to attach

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Actual Type declarations
/// Template args:
///   T: Storage Type
///   bits: first bit position
///   Sz: Number of storage bits
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T, size_t bits, size_t Sz>
struct LEItemType;  // storage type for Little Endian items

template<typename T, size_t bits, size_t Sz, size_t N>
struct LEArrayType;  // storage type for Little Endian Arrays

template<size_t bits>
struct BoolItemType;  // storage type for single bool

template<size_t bits, size_t N>
struct BoolArrayType;  // storage type for bool Arrays

template<size_t bits, size_t AlignByte>
struct AlignPadType;  // dummy storage for byte alignment


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Vstruct declaration is needed as we want to attach to the base
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct VStruct{
 public:
  pbuf_type* internal_buf_;
  pbuf_type* getBuffer() {
    return internal_buf_;
  }
  void setBuffer(pbuf_type* pBuffer) {
    internal_buf_ = pBuffer;
  }
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Little Endian Integer / Float
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T, size_t bits, size_t Sz>
struct LEItemType final : public internals::TypeBase<T, bits, Sz, 1> {
  static_assert(!std::is_base_of<bool, T>::value, "bool type is not allowed, use BoolItem instead");
  static_assert(!std::is_floating_point<T>::value ||(std::is_floating_point<T>::value && (Sz == (sizeof(T) << 3))),
                "No compression allowed for floating point types, Sz must match floating point sizeof");
  static_assert(Sz > 0, "Size must be 1 or more");
  static_assert(Sz <= 64, "Maximum 64bit _ItemBase supported");
  static_assert(Sz <= 8 * sizeof(T), "Bit packed _ItemBase should be equal or less than Raw _ItemBase");

  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit LEItemType(pbuf_type* &pbuf): pbuf_(pbuf) {}
  // NOLINTNEXTLINE(runtime/references)
  explicit LEItemType(VStruct &baseStruct): pbuf_(baseStruct.internal_buf_) {}

  operator T() const {  // getter
      return internals::Packer<T, Sz>::unpack(
               internals::LEOrder<typename LEItemType::packedT, Sz>::get(pbuf_, bits));
  }

  LEItemType<T, LEItemType::b, Sz>& operator= (const T& value) {  // setter
      internals::LEOrder<typename LEItemType::packedT, Sz>::set(
        pbuf_, bits, internals::Packer<T, Sz>::pack(value));
  }
};

template<typename T, size_t bits, size_t Sz, size_t N>
struct LEArrayType final : public internals::TypeBase<T, bits, Sz, N> {
  static_assert(!std::is_base_of<T, bool>::value, "bool type is not allowed");
  static_assert(Sz > 0, "Size must be 1 or more");
  static_assert(Sz <= 64, "Maximum 64bit supported");
  static_assert(Sz <= 8 * sizeof(T), "Bit packed should be equal or less than original type");

  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit LEArrayType(pbuf_type* &pbuf): pbuf_(pbuf) {}
  // NOLINTNEXTLINE(runtime/references)
  explicit LEArrayType(VStruct &baseStruct): pbuf_(baseStruct.internal_buf_) {}

  // index operator is exposed. returns the temporary array object
  internals::LEArrayTemp<T, Sz> operator[](size_t index) {
    return internals::LEArrayTemp<T, Sz>{&pbuf_[LEArrayType::B], LEArrayType::bits + index * LEArrayType::Sz};
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Bool Types
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t bits>
struct BoolItemType final : public internals::TypeBase<bool, bits, 1, 1> {
  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit BoolItemType(pbuf_type* &pbuf): pbuf_(pbuf) {}
  // NOLINTNEXTLINE(runtime/references)
  explicit BoolItemType(VStruct &baseStruct): pbuf_(baseStruct.internal_buf_) {}

  operator bool () const {
    return vstruct::internals::ByteAccess<1, BoolItemType::b>::get(&pbuf_[BoolItemType::B]);
  }

  BoolItemType<BoolItemType::b>& operator= (const bool& value) {
    vstruct::internals::ByteAccess<1, BoolItemType::b>::set(&pbuf_[BoolItemType::B], static_cast<uint8_t>(value));
  }
};

template<size_t bits, size_t N>
struct BoolArrayType final : public internals::TypeBase<bool, bits, 1, N> {
  static_assert(N > 0, "Size must be 1 or more");
  pbuf_type* &pbuf_;
  // Google Style-guide disallows non-const reference for API, we need this
  // NOLINTNEXTLINE(runtime/references)
  explicit BoolArrayType(pbuf_type* &pbuf): pbuf_(pbuf) {}
  // NOLINTNEXTLINE(runtime/references)
  explicit BoolArrayType(VStruct &baseStruct): pbuf_(baseStruct.internal_buf_) {}

  internals::BoolArrayTemp<BoolArrayType::b, N> operator[](size_t index) {
    return internals::BoolArrayTemp<BoolArrayType::b, N>{ &pbuf_[BoolArrayType::B], index};
  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Padding for alignment
////////////////////////////////////////////////////////////////////////////////////////////////////////
template<size_t bits, size_t AlignByte>
struct AlignPadType final {
  static_assert(AlignByte <= 8, "Maximum 8 byte allignment allowed");
  enum : size_t {
    misalignment = (bits % (AlignByte * 8)),
    next_bit = (misalignment > 0) ? bits + ((AlignByte * 8) - misalignment): bits
  };
  explicit AlignPadType(){}
};

template<typename Prev, typename T, size_t Sz>
struct LEItem {
  using type = LEItemType<T, Prev::next_bit, Sz>;
  LEItem() = delete;
};

template<typename Prev, typename T, size_t Sz, size_t N>
struct LEArray {
  using type = LEArrayType<T, Prev::next_bit, Sz, N>;
  LEArray() = delete;
};

template<typename Prev>
struct BoolItem {
  using type = BoolItemType<Prev::next_bit>;
  BoolItem() = delete;
};

template<typename Prev, size_t N>
struct BoolArray{
  using type = BoolArrayType<Prev::next_bit, N>;
  BoolArray() = delete;
};

template<typename Prev, size_t AlignByte>
struct AlignPad{
  using type = AlignPadType<Prev::next_bit, AlignByte>;
};

struct Root final {
  enum : size_t {
    next_bit = 0
  };
  Root() = delete;
};



}  // namespace vstruct


#endif  // VSTRUCT_ITEMTYPES_H_
