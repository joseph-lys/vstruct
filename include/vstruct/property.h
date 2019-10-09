#include "internals.h"

namespace  vstruct {


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


} /* namespace vstruct */
