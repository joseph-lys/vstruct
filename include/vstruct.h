/* vstruct.h

Virtual Struct to hold odd sized members.




Example Usage:

#include "vstruct.h"

class MyStruct : vstruct::VStruct
{
  // single item named "itemA", unpacked type is "int", packed size is 6 bits
  vstruct::Item<int, 6> itemA {&this};  

  // array named "arrayB", unpacked type is "unsigned int", packed size is 15 bits, 7 array items
  vstruct::Array<unsigned int, 15> arrayB {&this, 7}
};

uint8_t* bar = new uint8_t[100];
MyStruct foo;

foo.setBuffer(bar);
foo.itemA = 1;  // store value 1 into bar
foo.arrayB[1] = 2;  // store value 1 into bar



*/

#ifndef VSTRUCT_H_
#define VSTRUCT_H_


#include "vstruct/internals.h"
#include "vstruct/vstructbase.h"
#include "vstruct/indexbase.h"
#include "vstruct/itembase.h"
#include "vstruct/arraybase.h"



#endif
