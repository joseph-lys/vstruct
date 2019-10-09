#ifndef VSTRUCT_ACCESS_METHODS_H_
#define VSTRUCT_ACCESS_METHODS_H_


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

        if(x > max_val)
        {
          x = max_val;
        }
        else if(std::is_signed<T>::value && (x < min_val))
        {
          x = min_val;
        }
        return (Tpacked)x;
      }
    };



  } /* namespace _internals*/

  template <typename T, uint16_t Sz>
  struct PropertyIF
  {
    typedef typename std::conditional<std::is_signed<T>::value, typename std::make_unsigned<T>::type, T>::type Tpacked;

    static T getLE(uint8_t* buffer, uint16_t position_in_bits)
    {
      Tpacked x = _internals::RawIF<Tpacked, Sz>::getLE(buffer, position_in_bits);
      return _internals::Clip<T, Sz>::unpackSign(x);
    }

    static void setLE(uint8_t* buffer, uint16_t position_in_bits, T value)
    {
      Tpacked x = _internals::Clip<T, Sz>::packSign(value);
      _internals::RawIF<Tpacked, Sz>::setLE(buffer, position_in_bits, x);
    }
  };

  /*  specialization for bool types */
  template <>
  struct PropertyIF<bool, 1>
  {
    static bool getLE(uint8_t* buffer, uint16_t position_in_bits)
    {
      uint16_t B = position_in_bits >> 3;
      uint16_t b = position_in_bits & 7u;
      return (bool)(buffer[B] & (1u << b));
    }

    static void setLE(uint8_t* buffer, uint16_t position_in_bits, bool value)
    {
      uint16_t B = position_in_bits >> 3;
      uint16_t b = position_in_bits & 7u;
      if(value)
      {
        buffer[B] |= (1u << b);
      }
      else
      {
        buffer[B] &= ~(1u << b);
      }
    }
  };

  /*  error case for bool types */
  template <uint16_t Sz>
  struct PropertyIF<bool, Sz>
  {
    static_assert(true, "bool property only allowed to have Sz of 1");
  };

} /*namespace vstruct*/





#endif /* VSTRUCT_ACCESS_METHODS_H_ */

