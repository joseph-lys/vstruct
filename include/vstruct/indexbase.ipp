#ifndef VSTRUCT_INDEX_BASE_IPP_
#define VSTRUCT_INDEX_BASE_IPP_

#include "indexbase.h"
#include "vstructbase.h"

namespace vstruct{

  _IndexBase::_IndexBase(uint8_t* &buf, uint16_t position_in_bits, const uint16_t size_in_bits)
  : _buf(buf), _b(position_in_bits), _sz(size_in_bits)
  {}

  _IndexBase::_IndexBase(_VStructBase& s, const uint16_t size_in_bits)
  : _IndexBase(s._buf, s.bitSize(), size_in_bits) 
  {
    s.addBits(size_in_bits);
  }

  uint16_t _IndexBase::_IndexBase::totalBits()
  { 
    return _b + _sz; 
  }


} // namespace vstruct
#endif /* VSTRUCT_INDEX_BASE_IPP_ */