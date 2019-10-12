#ifndef VSTRUCT_ITEM_BASE_H_
#define VSTRUCT_ITEM_BASE_H_


#include <stdint.h>
#include "xvstructbase.h"
#include "internals.h"


namespace xvstruct{
/* *****************************************************************************************************
 * Struct declarations and access functions
 * ****************************************************************************************************/

  template<typename T, uint16_t b, uint16_t Sz>
  struct LEItem;  // storage class for Little Endian items

  template<typename T, uint16_t b, uint16_t Sz, uint16_t N>
  struct LEArray;  // storage class for Little Endian Arrays

  template<uint16_t b>
  struct BoolItem;  // storage class for single bool

  template<uint16_t b, uint16_t N>
  struct BoolArray;  // storage class for bool Arrays

  template<uint16_t b, uint16_t AlignBit>
  struct AlignPad;  // dummy storage for byte alignment

  struct RootItem;  // dummy storage for root of struct


/* *****************************************************************************************************
 * Little Endian Integer / Float
 * ****************************************************************************************************/
  template<typename T, uint16_t b, uint16_t Sz>
  struct LEItem
  {
      static_assert(!std::is_base_of<bool, T>::value, "bool type is not allowed");
      static_assert(!std::is_floating_point<T>::value ||(std::is_floating_point<T>::value && (Sz == (sizeof(T) << 3))),
                    "No compression allowed for floating point types, Sz must match floating point sizeof");
      static_assert(Sz > 0, "Size must be 1 or more");
      static_assert(Sz <= 64, "Maximum 64bit _ItemBase supported");
      static_assert(Sz <= 8 * sizeof(T), "Bit packed _ItemBase should be equal or less than Raw _ItemBase");


      typedef typename internals::Clip<T, Sz>::packedT packedT;
      enum : uint16_t { next_bit = b + Sz }; // for next Item
      pbuf_type* &pbuf_;
      LEItem(pbuf_type* &pbuf): pbuf_(pbuf) {}

      operator T() const  //getter
      {
          return internals::Clip<T, Sz>::unpackSign(internals::RawIF<packedT, b, Sz>::getLE(pbuf_));
      }

      LEItem<T, b, Sz>& operator= (const T& value)  //setter
      {
          internals::RawIF<packedT, b, Sz>::setLE(pbuf_, internals::Clip<T, Sz>::packSign(value));
      }
  };


  // Actual object stored in the Struct
  template<typename T, uint16_t b, uint16_t Sz, uint16_t N>
  struct LEArray
  {
      static_assert(!std::is_base_of<T, bool>::value, "bool type is not allowed");
      static_assert(!std::is_floating_point<T>::value ||(std::is_floating_point<T>::value && (Sz == (sizeof(T) << 3))),
                    "No compression allowed for floating point types, Sz must match floating point sizeof");
      static_assert(Sz > 0, "Size must be 1 or more");
      static_assert(Sz <= 64, "Maximum 64bit supported");
      static_assert(Sz <= 8 * sizeof(T), "Bit packed should be equal or less than original type");

      enum : uint16_t { next_bit = b + (Sz * N) }; // for next Item
      pbuf_type* &pbuf_;
      LEArray(pbuf_type* &pbuf): pbuf_(pbuf) {}

      /* index operator is exposed. returns the temporary array object */
      internals::LEArrayTemp<T, b, Sz, N> operator[](uint16_t index)
      {
        return internals::LEArrayTemp<T, b, Sz, N>{pbuf_, index};
      }
  };


  /* *****************************************************************************************************
   * Bool Types
   * ****************************************************************************************************/
  template<uint16_t b>
  struct BoolItem
  {
    enum : uint16_t { next_bit = b + 1 }; // for next Item
    pbuf_type* &pbuf_;
    BoolItem(pbuf_type* &pbuf): pbuf_(pbuf) {}

    enum : uint16_t
    {
      B_ = b >> 3,
      b_ = b & 7u
    };
    operator bool () const
    {
      return (bool)(pbuf_[B_] & (1u << b_));
    }

    BoolItem<b>& operator= (const bool& value)
    {
      if(value)
      {
        pbuf_[B_] |= (1u << b_);
      }
      else
      {
        pbuf_[B_] &= ~(1u << b_);
      }
    }
  };

  template<uint16_t b, uint16_t N>
  struct BoolArray
  {
    static_assert(N > 0, "Size must be 1 or more");
    enum : uint16_t { next_bit = b + N }; // for next Item
    pbuf_type* &pbuf_;
    BoolArray(pbuf_type* &pbuf): pbuf_(pbuf) {}

    internals::BoolArrayTemp operator[](int index)
    {
      return internals::BoolArrayTemp{pbuf_, b + index};
    }
  };


  /* *****************************************************************************************************
   * Padding for alignment
   * ****************************************************************************************************/
  template<uint16_t b, uint16_t AlignBit>
  struct AlignPad
  {
    static_assert(AlignBit <= 64, "Maximum 64 bit allignment allowed");
    enum : uint16_t { next_bit = internals::nextAlignedBit<b, AlignBit>::value }; // for next Item
    pbuf_type* &pbuf_;
    AlignPad(pbuf_type* &pbuf): pbuf_(pbuf) {}
  };


  /* *****************************************************************************************************
   * Root, To make things easier, starts at zero
   * ****************************************************************************************************/
  struct RootItem
  {
    enum : uint16_t { next_bit = 0 }; // for next Item
    pbuf_type* &pbuf_;
    RootItem(pbuf_type* &pbuf): pbuf_(pbuf) {}
  };

} // namespace vstruct

#endif /* VSTRUCT_ITEM_BASE_H_ */
