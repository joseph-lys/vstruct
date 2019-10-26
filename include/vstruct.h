/// vstruct.h
///
/// Copyright (c) 2019 Joseph Lee Yuan Sheng
///
/// This file is part of vstruct which is released under MIT license.
/// See LICENSE file or go to https://github.com/joseph-lys/vstruct for full license details.
///
///
/// Virtual Struct to hold odd sized members.
///
///
///
/// Example Usage:
///
/// #include "vstruct.h"
///
/// class MyStruct : vstruct::VStruct
/// {
///   // single item named "itemA", unpacked type is "int", packed size is 6 bits
///   vstruct::Item<int, 6> itemA {&this};
///
///   // array named "arrayB", unpacked type is "unsigned int", packed size is 15 bits, 7 array items
///   vstruct::Array<unsigned int, 15> arrayB {&this, 7}
/// };
///
/// uint8_t* bar = new uint8_t[100];
/// MyStruct foo;
///
/// foo.setBuffer(bar);
/// foo.itemA = 1;  // store value 1 into bar
/// foo.arrayB[1] = 2;  // store value 1 into bar
///
///

#ifndef VSTRUCT_H_
#define VSTRUCT_H_


#include "vstruct/internals.h"
#include "vstruct/itemtypes.h"

namespace vstruct {
struct VStruct{
  /// nothing here for now.
};



}  // namespace


#endif  //INCLUDE_VSTRUCT_H_
