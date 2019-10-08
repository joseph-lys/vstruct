#ifndef VSTRUCT_ACCESS_METHODS_IPP_
#define VSTRUCT_ACCESS_METHODS_IPP_

#include "accessmethods.h"
#include <limits>

namespace vstruct{


template <class uT, uint16_t Sz>
uT RawIF<uT, Sz>::getLE(uint8_t* buffer, uint16_t position_in_bits)
{
  uT x=0;
  uT mask;
  uT temp;

  /*
  if(Sz >= std::numeric_limits<uT>::digits)
    mask = std::numeric_limits<uT>::max();
  else
    mask = ((1u << Sz) - 1);
  */
  mask = MaskMax<uT, Sz>::value;

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

template <class T, uint16_t Sz>
void setter(uint8_t* buffer, uint16_t position_in_bits, T sx)
{
  typedef typename std::make_unsigned<T>::type uT;
  uT x = vstruct::Clip<T, Sz>::packSign(sx);
  vstruct::RawIF<uT, Sz>::setLE(buffer, position_in_bits, x);
}

template <class T, uint16_t Sz>
T getter(uint8_t* buffer, uint16_t position_in_bits)
{
  typedef typename std::make_unsigned<T>::type uT;
  uT x = vstruct::RawIF<T, Sz>::getLE(buffer, position_in_bits);
  return vstruct::Clip<T, Sz>::unpackSign(x);
}

/*  specialization for bool types */
template <>
bool getter<bool, 1>(uint8_t* buffer, uint16_t position_in_bits)
{
  uint16_t B = position_in_bits >> 3;
  uint16_t b = position_in_bits & 7u;
  return (bool)(buffer[B] & (1u << b));
}

/* _setter specialization for bool types*/
template <>
void setter<bool, 1>(uint8_t* buffer, uint16_t position_in_bits, bool x)
{
  uint16_t B = position_in_bits >> 3;
  uint16_t b = position_in_bits & 7u;
  if(x)
  {
    buffer[B] |= (1u << b);
  }
  else
  {
    buffer[B] &= ~(1u << b);
  }
}



} // namespace vstruct

#endif /* VSTRUCT_ACCESS_METHODS_IPP_ */
