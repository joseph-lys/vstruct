#ifndef VSTRUCT_ITEM_BASE_H_
#define VSTRUCT_ITEM_BASE_H_


#include <stdint.h>

#include "vstructbase.h"
#include "indexbase.h"
#include "accessmethods.h"


namespace vstruct{

  template <class T, uint16_t Sz>
  class _ItemBase : public _IndexBase
  {
    static_assert(Sz > 0, "Size must be 1 or more");
    static_assert(Sz <= 32, "Maximum 32bit _ItemBase supported");
    static_assert(Sz <= 8 * sizeof(T), "Bit packed _ItemBase should be equal or less than Raw _ItemBase");
    
    public:
    _ItemBase() = delete;  // no default constructor
    _ItemBase( const _ItemBase<T, Sz>& ) = default; // default copy constructor
    //_ItemBase( const _ItemBase<T, Sz>&& ) = default; // default move constructor

    // intialize an item instance at relative bit position to buffer
    _ItemBase(uint8_t* &buf, uint16_t position_in_bits);

    // intialize an item instance at next bit poisition in VStruct
    _ItemBase(_VStructBase& s);
    
    // set data from another item
    _ItemBase<T, Sz>& operator=( const _ItemBase<T, Sz>& );  

    // set the data from unpacked data
    _ItemBase<T, Sz>& operator= (const T& value);

    // get the data
    operator T () const; // getter;}
  };

  // expose as vstruct::Item
  template <class T, uint16_t Sz> 
  using Item = _ItemBase<T, Sz>; 


  template <class T, uint16_t Sz> 
  _ItemBase<T, Sz>::_ItemBase(uint8_t* &buf, uint16_t position_in_bits)
  : _IndexBase(buf, position_in_bits, Sz)
  {}

  template <class T, uint16_t Sz> 
  _ItemBase<T, Sz>::_ItemBase(struct _VStructBase& s)
  : _IndexBase(s, Sz)
  {}

  template <class T, uint16_t Sz> 
  _ItemBase<T, Sz>& _ItemBase<T, Sz>::operator=( const _ItemBase<T, Sz>& other) 
  {
    T x = other;
    PropertyIF<T, Sz>::setLE(_buf, _b, x);
  }

  template <class T, uint16_t Sz> 
  _ItemBase<T, Sz>& _ItemBase<T, Sz>::operator= (const T& value)
  { 
    PropertyIF<T, Sz>::setLE(_buf, _b, value);
  }

  template <class T, uint16_t Sz> 
  _ItemBase<T, Sz>::operator T () const
  { 
    return PropertyIF<T, Sz>::getLE(_buf, _b);
  }



} // namespace vstruct

#endif /* VSTRUCT_ITEM_BASE_H_ */
