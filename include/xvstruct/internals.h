#ifndef VSTRUCT_INTERNALS_H_
#define VSTRUCT_INTERNALS_H_


#include <stdint.h>
#include <limits>
#include <type_traits>

namespace vstruct
{
  namespace _internals
  {
    // iteration function for generating maximum value
    template <typename T, uint16_t b, uint16_t Sz>
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

    template <typename T, uint16_t b, uint16_t Sz>
    struct Clip
    {
      typedef typename std::conditional<std::is_signed<T>::value, typename std::make_unsigned<T>::type, T>::type Tpacked;

      static T unpackSign(Tpacked x)
      {
        Tpacked max_val = (Tpacked)MaskMax<T, Sz>::value;
        Tpacked min_val = ~max_val;
        if(std::is_signed<T>::value && (min_val & x))
        {
          x |= min_val;
        }
        return (T)x;
      }

      static Tpacked packSign(T x)
      {
        Tpacked max_val = (Tpacked)MaskMax<T, Sz>::value;
        Tpacked min_val = ~max_val;

        if(x > (T)max_val)
        {
          x = max_val;
        }
        else if(std::is_signed<T>::value && (x < (T)min_val))
        {
          x = min_val;
        }
        return (Tpacked)x;
      }
    };

    // templated iterator for getting Little Endian
    template <typename T, uint16_t B, uint16_t forArg, uint16_t forArgLT, uint16_t offset>
    T forLEGet(uint8_t * buffer, T x)
    {
      if(forArg < forArgLT)
      {
        x |= (buffer[B + forArg]) << ((forArg << 3) - offset);
        return forLEGet<T, B, forArg + 1, forArgLT, offset>(buffer, x);
      }
      return x;
    }

    template <typename T, uint16_t pb, uint16_t Sz>
    T RawIF<T, Sz>::getLE(uint8_t* buffer)
    {
      T x=0;
      enum: uint16_t{
        B = pb >> 3,
        b = pb & 7u,
        total_bytes = (b + Sz + 7) >> 3
      };

      if(b == 0 and Sz >= 8) // no offset, no mask
      {
        x = forLEGet<0, total_bytes, 0>(buffer, x);
      }
      else // with offset
      {
        x = buffer[B] >> b;
        x = forLEGet<1, total_bytes, b>(buffer, x);
      }
      return x &= MaskMax<T, Sz>::value;
    }

    // templated iterator for setting Little Endian
    template <typename T, uint16_t B, uint16_t forArg, uint16_t forArgLT, uint16_t offset>
    void forLESet(uint8_t * buffer, T x)
    {
      if(forArg < forArgLT)
      {
        buffer[B + i] = x >> ((forArg << 3) - offset);
        forLESet<T, B, forArg + 1, forArgLT, offset>(buffer, x);
      }
    }

    template <typename T, uint16_t pb, uint16_t Sz>
    void RawIF<uT, Sz>::setLE(uint8_t* buffer, T x)
    {
      uT mask = MaskMax<uT, Sz>::value;
      x &= mask;
      enum : uint16_t
      {
        B = position_in_bits >> 3,
        b = position_in_bits & 7u,
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
        if((sZ + b) & 7) // if last bit is not aligned to 8, must use mask
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

  } /* namespace _internals*/


} /*namespace vstruct*/





#endif /* VSTRUCT_INTERNALS_H_ */

