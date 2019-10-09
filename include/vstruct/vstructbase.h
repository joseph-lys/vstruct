#ifndef VSTRUCT_BASE_H_
#define VSTRUCT_BASE_H_


#include <stdint.h>


namespace vstruct{
  
  class _IndexBase;

  // Base class to attach items
  struct _VStructBase
  {
    friend class _IndexBase;
    uint8_t* _buf;
    uint16_t _total_bits = 0;
    uint16_t addBits(uint16_t bits);
    public:

    // returns total bytes used
    uint16_t byteSize() const;
    
    // returns total bits used
    uint16_t bitSize() const;

    // set the read/write target to a different buffer
    void setBuffer(uint8_t* new_buffer);
    
  };

  // expose as VStruct type
  typedef _VStructBase VStruct; 



} /* namespace vstruct */



uint16_t vstruct::_VStructBase::addBits(uint16_t bits)
{
  _total_bits += bits;
  return bits;
}

uint16_t vstruct::_VStructBase::byteSize() const 
{ 
  return (_total_bits + 7u) >> 3;
}

uint16_t vstruct::_VStructBase::bitSize() const 
{ 
  return _total_bits;
}

void vstruct::_VStructBase::setBuffer(uint8_t* new_buffer)
{
  _buf = new_buffer;
}

#endif /* VSTRUCT_BASE_H_ */