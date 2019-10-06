#ifndef VSTRUCT_ACCESS_METHODS_IPP_
#define VSTRUCT_ACCESS_METHODS_IPP_

#include "accessmethods.h"

namespace vstruct{
template <class T, uint16_t Sz>
T _unpackSign(T x)
{
  if(std::is_signed<T>::value)
  {
    T smask =(T)(1u << (Sz - 1));
    if(smask & x)
    {
      x |= ~(smask - 1);
    }
    else
    {
      x &= smask - 1;
    }
    
  }
  return x;
}

template <class T, uint16_t Sz>
T _packSign(T x)
{
  if(std::is_signed<T>::value)
  {
    T smask = 1u << (Sz - 1);
    if(x < 0)
    {
      x |= ~(smask - 1);
    }
    else
    {
      x &= smask - 1;
    }
  }
  else
  {
    if(x >= (1u << Sz))
    {
      x = (1u << Sz) - 1;
    }
  }
  
  return x;
}

template <class T, uint16_t Sz>
T _getter(uint8_t* buffer, uint16_t position_in_bits)
{
  typedef typename std::make_unsigned<T>::type uT;
  uT x=0;
  uint16_t B = position_in_bits >> 3;
  uint16_t b = position_in_bits & 7u;
  if((Sz & 7u) == 0 && b == 0) // no masking needed
  {
    if(Sz == 8)
    {
      x = buffer[B];
    }
    else if(Sz == 16)
    {
      x = buffer[B + 1] << 8;
      x |= buffer[B];
    }
    else if(Sz == 24)
    {
      x = buffer[B + 2] << 16;
      x |= buffer[B + 1] << 8;
      x |= buffer[B];
    }
    else if(Sz == 32)
    {
      x = buffer[B + 3] << 24;
      x |= buffer[B + 2] << 16;
      x |= buffer[B + 1] << 8;
      x |= buffer[B];
    }
  }
  else // use mask
  {
    uT mask = ((1u << Sz) - 1);
    if(Sz <= 8  && Sz + b <= 8)
    {
      x = buffer[B] >> b; 
    }
    else if(Sz <= 16  && Sz + b <= 16)
    {
      x = buffer[B + 1] << (8 - b);
      x |= buffer[B] >> b;
    }
    else if(Sz > 8 && Sz <= 24 && Sz + b <= 24)
    {
      x = buffer[B + 2] << (16 - b);
      x |= buffer[B + 1] << (8 - b);
      x |= buffer[B] >> b;
    }
    else if(Sz > 16 && Sz <= 32 && Sz + b <= 32)
    {
      x =  buffer[B + 3] << (24 - b); 
      x |= buffer[B + 2] << (16 - b);
      x |= buffer[B + 1] << (8 - b);
      x |= buffer[B] >> b;
    }
    else if(Sz > 24 && Sz <= 32 && Sz + b <= 40)
    {
      x = buffer[B + 4] << (32 - b); 
      x |= buffer[B + 3] << (24 - b);
      x |= buffer[B + 2] << (16 - b);
      x |= buffer[B + 1] << (8 - b);
      x |= buffer[B] >> b;
    }
    x &= mask;
  }
  return (T)x;
}

template <class T, uint16_t Sz>
void _setter(uint8_t* buffer, uint16_t position_in_bits, T _x)
{ 
  typedef typename std::make_unsigned<T>::type uT;
  uT x= (uT)_x;
  uint16_t B = position_in_bits >> 3;
  uint16_t b = position_in_bits & 7u;
  if((Sz & 7u) == 0 && b == 0) // no masking needed
  {
    if(Sz == 8)
    {
      buffer[B] = x;
    }
    else if(Sz == 16)
    {
      buffer[B + 1] = x >> 8;
      buffer[B] = x;
    }
    else if(Sz == 24)
    {
      buffer[B + 2] = x >> 16;
      buffer[B + 1] = x >> 8;
      buffer[B] = x;
    }
    else if(Sz == 32)
    {
      buffer[B + 3] = x >> 24;
      buffer[B + 2] = x >> 16;
      buffer[B + 1] = x >> 8;
      buffer[B] = x;
    }
  }
  else // use mask
  {
    uT mask = ((1u << Sz) - 1);
    x = x & mask;
    if(Sz <= 8  && Sz + b <= 8)
    {
      buffer[B] &= ~mask << b;
      buffer[B] |= (x << b) & 0xff;
    }
    else if(Sz <= 16  && Sz + b <= 16)
    {
      buffer[B + 1] &= ~mask >> (8 - b);
      buffer[B + 1] |= (x >> (8 - b)) & 0xff;
      buffer[B] &= ~mask << b;
      buffer[B] |= (x << b) & 0xff;
    }
    else if(Sz > 8 && Sz <= 24 && Sz + b <= 24)
    {
      buffer[B + 2] &= ~mask >> (16 - b);
      buffer[B + 2] |= (x >> (16 - b)) & 0xff;
      buffer[B + 1] = (x >> (8 - b)) & 0xff; 
      buffer[B] &= ~mask << b;
      buffer[B] |= (x << b) & 0xff;
    }
    else if(Sz > 16 && Sz <= 32 && Sz + b <= 32)
    {
      buffer[B + 3] &= ~mask >> (24 - b);
      buffer[B + 3] |= (x >> (24 - b)) & 0xff;
      buffer[B + 2] = (x >> (16 - b)) & 0xff; 
      buffer[B + 1] = (x >> (8 - b)) & 0xff; 
      buffer[B] &= ~mask << b;
      buffer[B] |= x << b;
    }
    else if(Sz > 24 && Sz <= 32 && Sz + b <= 40)
    {
      buffer[B + 4] &= ~mask >> (32 - b);
      buffer[B + 4] |= x >> (32 - b); 
      buffer[B + 3] = x >> (24 - b); 
      buffer[B + 2] = x >> (16 - b); 
      buffer[B + 1] = x >> (8 - b); 
      buffer[B] &= ~mask << b;
      buffer[B] |= x << b;
    }
  }
}

template <class T, uint16_t Sz>
void setter(uint8_t* buffer, uint16_t position_in_bits, T x)
{
  x = _packSign<T, Sz>(x);
  _setter<T, Sz>(buffer, position_in_bits, x);  
}

template <class T, uint16_t Sz>
T getter(uint8_t* buffer, uint16_t position_in_bits)
{
  T x = (T) _getter<T, Sz>(buffer, position_in_bits);
  return _unpackSign<T, Sz>(x);
}

/*  specialization for bool types */
template <>
bool _getter<bool, 1>(uint8_t* buffer, uint16_t position_in_bits)
{
  uint16_t B = position_in_bits >> 3;
  uint16_t b = position_in_bits & 7u;
  return (bool)(buffer[B] & (1u << b));
}

/* _setter specialization for bool types*/
template <>
void _setter<bool, 1>(uint8_t* buffer, uint16_t position_in_bits, bool x)
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