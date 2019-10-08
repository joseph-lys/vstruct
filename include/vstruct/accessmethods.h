#ifndef VSTRUCT_ACCESS_METHODS_H_
#define VSTRUCT_ACCESS_METHODS_H_


#include <stdint.h>
#include <limits>
#include <type_traits>

namespace vstruct
{
  template <typename T, uint16_t Sz>
  struct Clip
  {
     typedef typename std::make_unsigned<T>::type uT;

     // Helper function for unpacking sign bits
     static T unpackSign(uT x);

     // Helper function forpacking sign bits
     static uT packSign(T x);


  };

  // class that makes the raw interface to buffer
  template <class uT, uint16_t Sz>
  struct RawIF
  {
    static_assert(
        std::is_integral<uT>::value && !std::is_same<uT, bool>::value,
        "This interface class can only deal with unsigned integer types");

    // Get Bits from buffer, Little Endian Ordering
    static uT getLE(uint8_t* buffer, uint16_t position_in_bits);

    // Write Bits from buffer, Little Endian Ordering
    static void setLE(uint8_t* buffer, uint16_t position_in_bits, uT value);
  };

  // Implementation of converting packed data to original unpacked type
  template <typename T, uint16_t Sz> T getter(uint8_t* buffer, uint16_t position_in_bits);

  // Implementation of converting original unapacked type to packed data
  template <typename T, uint16_t Sz> void setter(uint8_t* buffer, uint16_t position_in_bits, T x);

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

  template <typename T, uint16_t Sz>
  struct MaskMin
  {
    enum : T
    {
      value = std::is_signed<T>::value? ~MaskMax<T, Sz - 1>::value : 0,
    };
  };

} /*namespace vstruct*/


template <typename sT, uint16_t Sz>
sT vstruct::Clip<sT, Sz>::unpackSign(typename std::make_unsigned<sT>::type x)
{
  typedef typename std::make_unsigned<sT>::type uT;
  uT min_val = MaskMin<sT, Sz>::value;
  if(min_val & x)
  {
    x |= min_val;
  }
  return (sT)x;
}

template <typename sT, uint16_t Sz>
typename std::make_unsigned<sT>::type vstruct::Clip<sT, Sz>::packSign(sT x)
{
  typedef typename std::make_unsigned<sT>::type uT;
  sT max_val = MaskMax<sT, Sz>::value;
  sT min_val = MaskMin<sT, Sz>::value;

  if(x > max_val)
  {
    x = max_val;
  }
  else if(std::is_signed<sT>::value && x < min_val)
  {
    x = min_val;
  }
  return (uT)x;
}




#endif /* VSTRUCT_ACCESS_METHODS_H_ */

