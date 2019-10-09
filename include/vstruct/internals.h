#ifndef VSTRUCT_INTERNALS_H_
#define VSTRUCT_INTERNALS_H_


#include <stdint.h>
#include <limits>
#include <type_traits>

namespace vstruct
{
  namespace _internals
  {

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
    template <class T, uint16_t Sz>
    struct RawIF
    {
      static_assert(
          std::is_integral<T>::value && std::is_unsigned<T>::value && !std::is_same<T, bool>::value,
          "This interface class can only deal with unsigned integer types");

      // Get Bits from buffer, Little Endian Ordering
      static T getLE(uint8_t* buffer, uint16_t position_in_bits);

      // Write Bits from buffer, Little Endian Ordering
      static void setLE(uint8_t* buffer, uint16_t position_in_bits, T value);
    };

    template <typename T, uint16_t Sz>
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


    template <typename T, uint16_t Sz>
    T RawIF<T, Sz>::getLE(uint8_t* buffer, uint16_t position_in_bits)
    {
      T x=0;
      T mask;
      T temp;

      /*
      if(Sz >= std::numeric_limits<uT>::digits)
        mask = std::numeric_limits<uT>::max();
      else
        mask = ((1u << Sz) - 1);
      */
      mask = MaskMax<T, Sz>::value;

      uint16_t B = position_in_bits >> 3;
      uint16_t b = position_in_bits & 7u;
      uint16_t total_bytes = (b + Sz + 7) >> 3;
      uint16_t i;
      if(b == 0) // no offset
      {
        for(i=0; i < total_bytes; i++)
        {
          temp = buffer[B + i];
          x |= temp << (i << 3);
        }
      }
      else // with offset
      {
        x = buffer[B] >> b;
        for(i=1; i < total_bytes; i++)
        {
          temp = buffer[B + i] ;
          x |= temp << ((i << 3) - b);
        }
      }
      return x &= mask;
    }

    template <class uT, uint16_t Sz>
    void RawIF<uT, Sz>::setLE(uint8_t* buffer, uint16_t position_in_bits, uT x)
    {
      uT mask = MaskMax<uT, Sz>::value;
      x &= mask;
      uint16_t B = position_in_bits >> 3;
      uint16_t b = position_in_bits & 7u;
      uint16_t last_byte = ((b + Sz - 1) >> 3);
      uint16_t i;
      // first byte
      buffer[B] &= ~(mask << b);
      buffer[B] |= (x << b);

      if(last_byte > 0)
      {
        // middle bytes
        for(i=1; i<last_byte; i++)
        {
          buffer[B + i] = x >> ((i << 3) - b);
        }

        // last byte
        buffer[B + last_byte] &= ~(mask >> ((last_byte << 3) - b));
        buffer[B + last_byte] |= x >> ((last_byte << 3) - b);
      }
    }

  } /* namespace _internals*/




} /*namespace vstruct*/





#endif /* VSTRUCT_INTERNALS_H_ */

