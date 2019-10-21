// todo: some copyright here
//
// xvstruct.h
//
//
// todo: description here
//

#ifndef INCLUDE_XVSTRUCT_H_
#define INCLUDE_XVSTRUCT_H_

#include <stdint.h>
#include "xvstruct/internals.h"
#include "xvstruct/itembase.h"


namespace xvstruct {


struct XVStruct {
  pbuf_type* pbuf_;
 public:
  // set the read/write target to a different buffer
  void setBuffer(pbuf_type* new_buffer);
};


////////////////////////////////////////////////////////////////////////////////////////////////////////
// XVStruct function definitions
////////////////////////////////////////////////////////////////////////////////////////////////////////
void XVStruct::setBuffer(uint8_t* new_buffer) {
  pbuf_ = new_buffer;
}

template<typename T, uint16_t Sz, typename prevT> LEItem<T, prevT::next_bit, Sz>
addLEItem(prevT previous_item) {
  return LEItem<T, prevT::next_bit, Sz>{previous_item.pbuf_};
}

// add an array of [N] Little Endian Items
template<typename T, uint16_t Sz, uint16_t N, typename prevT> LEArray<T, prevT::next_bit, Sz, N>
addLEArray(prevT previous_item) {
  return LEArray<T, prevT::next_bit, Sz, N>{previous_item.pbuf_};
}

// add a single bool item
template<typename prevT> BoolItem<prevT::next_bit>
addBoolItem(prevT previous_item) {
  return BoolItem<prevT::next_bit>{previous_item.pbuf_};
}

// add an array of [N] bool Items
template<uint16_t N, typename prevT> BoolArray<prevT::next_bit, N>
addBoolArray(prevT previous_item) {
  return BoolArray<prevT::next_bit, N>{previous_item.pbuf_};
}

// add some padding to align to [Pad] bytes
template<uint16_t Pad, typename prevT> AlignPad<prevT::next_bit, Pad>
addAlignPad(prevT previous_item) {
  return AlignPad<prevT::next_bit, Pad>{previous_item.pbuf_};
}




}  // namespace xvstruct


#endif  // INCLUDE_XVSTRUCT_H_
