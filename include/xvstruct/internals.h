#ifndef VSTRUCT_INTERNALS_H_
#define VSTRUCT_INTERNALS_H_


#include <stdint.h>
#include <limits>
#include <type_traits>

namespace xvstruct
{
  typedef uint8_t pbuf_type;

  namespace internals
  {
    // iteration function for generating maximum value
    template <typename T, uint16_t Sz>
    struct MaskMax
    {
      enum : T
      {
        value = std::is_signed<T>::value? MaskMax<typename std::make_unsigned<T>::type, Sz - 1>::value :(MaskMax<T, Sz - 1>::value << 1) | 1,
      };
    };

    template <typename T>
    struct MaskMax<T, 0>
    {
      enum : T
      {
        value = 0,
      };
    };


    // class that makes the raw interface to buffer
    template <class T, uint16_t b, uint16_t Sz>
    struct RawIF
    {
      static_assert(
          std::is_integral<T>::value && std::is_unsigned<T>::value && !std::is_same<T, bool>::value,
          "This interface class can only deal with unsigned integer types");

      // Get Bits from buffer, Little Endian Ordering
      static T getLE(uint8_t* buffer);

      // Write Bits from buffer, Little Endian Ordering
      static void setLE(uint8_t* buffer, T value);
    };

    template <typename T, uint16_t Sz>
    struct Clip
    {
      typedef typename std::conditional<std::is_signed<T>::value && !std::is_floating_point<T>::value,
                                        typename std::make_unsigned<T>::type, T>::type packedT;

      static T unpackSign(packedT x)
      {
        packedT max_val = (packedT)MaskMax<T, Sz>::value;
        packedT min_val = ~max_val;
        if(std::is_floating_point<T>::value)
        {
          // dont touch the floating point
        }
        else if(std::is_signed<T>::value && !std::is_floating_point<T>::value && (min_val & x))
        {
          x |= min_val;
        }
        return (T)x;
      }

      static packedT packSign(T x)
      {
        packedT max_val = (packedT)MaskMax<T, Sz>::value;
        packedT min_val = ~max_val;

        if(std::is_floating_point<T>::value)
        {
          // dont touch the floating point
        }
        else if(x > (T)max_val)
        {
          x = max_val;
        }
        else if(std::is_signed<T>::value && (x < (T)min_val))
        {
          x = min_val;
        }
        return (packedT)x;
      }
    };

    // templated iterator for getting Little Endian
    template <typename T, uint16_t B, uint16_t forArg, uint16_t forArgLT, uint16_t offset>
    T forLEGet(uint8_t * buffer, T x)
    {
      if(forArg < forArgLT)
      {
        x |= (buffer[B + forArg]) << ((forArg * 8) - offset);
        return forLEGet<T, B, forArg + 1, forArgLT, offset>(buffer, x);
      }
      return x;
    }

    template <typename T, uint16_t pb, uint16_t Sz>
    T RawIF<T, pb, Sz>::getLE(uint8_t* buffer)
    {
      T x=0;
      enum: uint16_t{
        B = pb >> 3,
        b = pb & 7u,
        total_bytes = (b + Sz + 7) >> 3
      };

      if(b == 0 and Sz >= 8) // no offset, no mask
      {
        x = forLEGet<T, B, 0, total_bytes, 0>(buffer, x);
      }
      else // with offset
      {
        x = buffer[B] >> b;
        x = forLEGet<T, B, 1, total_bytes, 0>(buffer, x);
      }
      return x &= MaskMax<T, Sz>::value;
    }

    // templated iterator for setting Little Endian
    template <typename T, uint16_t B, uint16_t forArg, uint16_t forArgLT, uint16_t offset>
    void forLESet(uint8_t * buffer, T x)
    {
      if(forArg < forArgLT)
      {
        buffer[B + forArg] = x >> ((forArg * 8) - offset);
        forLESet<T, B, forArg + 1, forArgLT, offset>(buffer, x);
      }
    }

    template <typename T, uint16_t pb, uint16_t Sz>
    void RawIF<T, pb, Sz>::setLE(uint8_t* buffer, T x)
    {
      T mask = MaskMax<T, Sz>::value;
      x &= mask;
      enum : uint16_t
      {
        B = pb >> 3,
        b = pb & 7u,
        last_byte = ((b + Sz - 1) >> 3)
      };
      // first byte
      if(Sz < 8 || b > 0) // if first bit does not fully fill byte or is not aligned, must use bit mask
      {
        buffer[B] &= ~(mask << b);
        buffer[B] |= (x << b);
      }
      else // direct copy
      {
        buffer[B] = x;
      }

      if(last_byte > 0)
      {
        // middle bytes
        forLESet<T, B, 1, last_byte, b>(buffer, x);

        // last byte
        if((Sz + b) & 7) // if last bit is not aligned to 8, must use mask
        {
          buffer[B + last_byte] &= ~(mask >> ((last_byte << 3) - b));
          buffer[B + last_byte] |= x >> ((last_byte << 3) - b);
        }
        else // bits are aligned to end of byte, can just copy
        {
          buffer[B + last_byte] = x >> (last_byte << 3);
        }
      }
    }

    // helper function to find next matching bit alignment
    template<uint16_t b, uint16_t AlignBit>
    struct nextAlignedBit
    {
      enum : uint16_t
      {
        value = b + (AlignBit - (b % AlignBit))
      };
    };

    //
    /* route according to last set lookup_ variable */
    template<typename T, uint16_t b, uint16_t Sz, uint16_t I>
    struct IndexedLE
    {
      static T getLE(pbuf_type* pbuf, int index)
      {
        switch (index) {
        case I:
          return RawIF<T, b, Sz>::getLE(pbuf);
        default:
          return IndexedLE<T, b, Sz, I - 1>::getLE(pbuf, index);
        }
      }
      static void setLE(pbuf_type* pbuf, int index, T value)
      {
        switch (index) {
        case I:
          return RawIF<T, b, Sz>::setLE(pbuf, value);
        default:
          return IndexedLE<T, b, Sz, I - 1>::setLE(pbuf, index, value);
        }
      }
    };

    template<typename T, uint16_t b, uint16_t Sz>
    struct IndexedLE <T, b, Sz, 0>
    {
      static T getLE(pbuf_type* pbuf, int index)
      {
        switch (index) {
        case 0:
          return RawIF<T, b, Sz>::getLE(pbuf);
        default:
          return 0;  // Todo: no error or an assert?
        }
      }
      static void setLE(pbuf_type* pbuf, int index, T value)
      {
        switch (index) {
        case 0:
          return RawIF<T, b, Sz>::setLE(pbuf, value);
        default:
          return;
        }
      }
    };

    // Temporary Object used by array
    template<typename T, uint16_t b, uint16_t Sz, uint16_t N>
    struct LEArrayTemp
    {
      typedef typename internals::Clip<T, Sz>::packedT packedT;
      pbuf_type* &pbuf_;
      const uint16_t index_;
      LEArrayTemp(pbuf_type* &pbuf, uint16_t index): pbuf_(pbuf),index_(index) {}

      /* getter and setter */
      operator T () const {
        packedT x = IndexedLE<packedT, b, Sz, N>::getLE(pbuf_, index_);
        return Clip<T, Sz>::unpackSign(x);
      }

      IndexedLE<T, b, Sz, N>& operator= (const T& value)
      {
        packedT x = Clip<T, Sz>::packSign(value);
        IndexedLE<packedT, b, Sz, N>::setLE(pbuf_, index_, x);
      }
    };


    struct BoolArrayTemp
    {
      pbuf_type* &pbuf_;
      const uint16_t B_;
      const uint16_t b_;
      BoolArrayTemp(pbuf_type* &pbuf, uint16_t b):
        pbuf_(pbuf),
        B_(b>> 3),
        b_(b & 7u)
        {}
      operator bool () const
      {
        return (bool)(pbuf_[B_] & (1u << b_));
      }

      BoolArrayTemp& operator= (const bool& value)
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

  } /* namespace _internals*/


} /*namespace vstruct*/





#endif /* VSTRUCT_INTERNALS_H_ */

