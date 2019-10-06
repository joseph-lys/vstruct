#ifndef VSTRUCT_ACCESS_METHODS_H_
#define VSTRUCT_ACCESS_METHODS_H_


#include <stdint.h>

namespace vstruct
{  
  // Helper function for unpacking sign bits
  template <class T, uint16_t Sz> T _unpackSign(T x);

  // Helper function forpacking sign bits
  template <class T, uint16_t Sz> T _packSign(T x);

  // Implementation of converting packed data to original unpacked type
  template <class T, uint16_t Sz> T _getter(uint8_t* buffer, uint16_t position_in_bits);

  // Implementation of converting original unapacked type to packed data
  template <class T, uint16_t Sz> void _setter(uint8_t* buffer, uint16_t position_in_bits, T x);

  // Implementation of converting packed data to original unpacked type
  template <class T, uint16_t Sz> T getter(uint8_t* buffer, uint16_t position_in_bits);

  // Implementation of converting original unapacked type to packed data
  template <class T, uint16_t Sz> void setter(uint8_t* buffer, uint16_t position_in_bits, T x);

} /*namespace vstruct*/


#endif /* VSTRUCT_ACCESS_METHODS_H_ */

