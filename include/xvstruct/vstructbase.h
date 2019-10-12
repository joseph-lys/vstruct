#ifndef VSTRUCT_BASE_H_
#define VSTRUCT_BASE_H_


#include <stdint.h>
#include "itembase.h"

namespace xvstruct{

  struct XVStruct
  {

    pbuf_type* pbuf_;
    public:

    // set the read/write target to a different buffer
    void setBuffer(pbuf_type* new_buffer);

    // for reference to first item
    static RootItem rootItem();

    // add a single Little Endian Item
    template<typename T, uint16_t Sz, typename prevT> LEItem<T, prevT::next_bit, Sz>
    static addLEItem(prevT previous_item);

    // add an array of [N] Little Endian Items
    template<typename T, uint16_t Sz, uint16_t N, typename prevT> LEArray<T, prevT::next_bit, Sz, N>
    static addLEArray(prevT previous_item);

    // add a single bool item
    template<typename prevT> BoolItem<prevT::next_bit>
    static addBoolItem(prevT previous_item);

    // add an array of [N] bool Items
    template<uint16_t N, typename prevT> BoolArray<prevT::next_bit, N>
    static addBoolArray(prevT previous_item);

    // add some padding to align to [Pad] bytes
    template<uint16_t Pad, typename prevT> AlignPad<prevT::next_bit, Pad>
    static addAlignPad(prevT previous_item);

  };


/* *****************************************************************************************************
 * XVStruct function definitions
 * ****************************************************************************************************/
  void XVStruct::setBuffer(uint8_t* new_buffer)
  {
    pbuf_ = new_buffer;
  }

  RootItem rootItem(XVStruct& instance)
  {
    return RootItem(instance.pbuf_);
  }

  template<typename T, uint16_t Sz, typename prevT> LEItem<T, prevT::next_bit, Sz>
  XVStruct::addLEItem(prevT previous_item)
  {
    return LEItem<T, prevT::next_bit, Sz>{previous_item.pbuf_};
  }

  // add an array of [N] Little Endian Items
  template<typename T, uint16_t Sz, uint16_t N, typename prevT> LEArray<T, prevT::next_bit, Sz, N>
  XVStruct::addLEArray(prevT previous_item)
  {
    return LEArray<T, prevT::next_bit, Sz, N>{previous_item.pbuf_};
  }

  // add a single bool item
  template<typename prevT> BoolItem<prevT::next_bit>
  XVStruct::addBoolItem(prevT previous_item)
  {
    return BoolItem<prevT::next_bit>{previous_item.pbuf_};
  }

  // add an array of [N] bool Items
  template<uint16_t N, typename prevT> BoolArray<prevT::next_bit, N>
  XVStruct::addBoolArray(prevT previous_item)
  {
    return BoolArray<prevT::next_bit, N>{previous_item.pbuf_};
  }

  // add some padding to align to [Pad] bytes
  template<uint16_t Pad, typename prevT> AlignPad<prevT::next_bit, Pad>
  XVStruct::addAlignPad(prevT previous_item)
  {
    return AlignPad<prevT::next_bit, Pad>{previous_item.pbuf_};
  }

} /* namespace vstruct */

#endif /* VSTRUCT_BASE_H_ */
