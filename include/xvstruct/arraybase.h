#ifndef VSTRUCT_ARRAY_BASE_H_
#define VSTRUCT_ARRAY_BASE_H_


#include <stdint.h>

#include "vstructbase.h"
#include "indexbase.h"
#include "itembase.h"


namespace vstruct{

  template <class T, uint16_t Sz>
  class _ArrayBase : public _IndexBase
  {
    private:
    const uint16_t _n;
    public:
    
    // intialize an array instance at relative bit position to buffer
    _ArrayBase(uint8_t* &buf, uint16_t position_in_bits, uint16_t array_size);

    // intialize an item instance at next bit poisition in VStruct
    _ArrayBase(_VStructBase& s, uint16_t array_size);
    
    // index operator is exposed.
    _ItemBase<T, Sz> operator[](int index);
  };

  // expose as vstruct::Array
  template <class T, uint16_t Sz> 
  using Array = _ArrayBase<T, Sz>; 

  template <class T, uint16_t Sz>
  _ArrayBase<T, Sz>::_ArrayBase(uint8_t* &buf, uint16_t position_in_bits, uint16_t array_size)
  : _IndexBase(buf, position_in_bits, Sz * array_size), _n(array_size)
  {}

  template <class T, uint16_t Sz>
  _ArrayBase<T, Sz>::_ArrayBase(_VStructBase& s, uint16_t array_size)
  : _IndexBase(s, Sz * array_size), _n(array_size)
  {}

  template <class T, uint16_t Sz>
  _ItemBase<T, Sz> _ArrayBase<T, Sz>::operator[](int index)
  {
      return _ItemBase<T, Sz>(_buf, _b + index * Sz);  
  }


} /* namespace vstruct */

#endif /* VSTRUCT_ARRAY_BASE_H_ */