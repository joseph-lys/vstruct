#include <iostream>
#include "xvstruct.h"

/*
 * Some example code on how to use vstruct.
 *
 * How to use
 * 1. Define a struct using vstruct::VStruct
 * 2. Populate with vstruct::Item or vstruct::Array templates
 * 3. vstruct::Item and vstruct::Array has to be initialized to the instance of vstruct::Vstruct
 *
*/

struct Foo : xvstruct::XVStruct
{


  /* template parameters for LEItem
   * are <[item type], [Number of bits]>
   *
   * the Item is attached to starting position of the internal buffer by calling rootItem()
   */
  decltype(addLEItem<int, 8>(rootItem())) first_item = addLEItem<int, 8>(rootItem());


  /* template parameters for LEArray
   * are <[item type], [Number of bits], [Array Size]>
   *
   * the Array is attached to buffer position right after the first_item
   */
  decltype(addLEArray<int, 7, 5>(first_item)) array_item = addLEArray<int, 7, 5>(first_item);


  /* template parameters for AlignPad
   * are <[Alignment Bits]>
   *
   * the padding is attached to buffer position right after the array_item
   */
  decltype(addAlignPad<16>(array_item)) padding = addAlignPad<16>(array_item);


  /* BoolItem does not require any parameter arguments
   *
   * the bit item is attached to buffer position right after the padding
   */
  decltype(addBoolItem(padding)) bit_value = addBoolItem(padding);


  /* template parameters for AlignPad
   * are <[Array Size]>
   *
   * the bit item is attached to buffer position right after the padding
   */
  decltype(addBoolArray<3>(bit_value)) bit_array = addBoolArray<3>(bit_value);

};

int main()
{
  Foo foo{};

  /* the vstruct needs to be attached to some memory
   * Here we allocate some memory from an array.
   * The VStruct has helper function to get the size.
   *
   * Note: You may want to zero intialize the memory
  */

  uint8_t some_memory[128]{0};
  foo.setBuffer(some_memory);
  // std::cout << "foo requires " << foo.byteSize() << " Bytes" << std::endl << std::endl;


  std::cout << "foo.item is " << foo.first_item << std::endl;
  for (int i=0; i<8; i++)
  {
    std::cout << "foo.array[" << i << "] is " << foo.array_item[i] << std::endl;
  }

  /* use the members as you would a normal interger struct member.
   * here are some assignments
  */

  foo.first_item = 1;
  foo.array_item[1] = 123;
  foo.array_item[2] = -123;
  foo.array_item[3] = 1000000;
  foo.array_item[4] = -1000000;

  /* Note:
   * 1. The array type has no boundary checking. MAKE Sure not to go over the assigned size!
   * 2. Values are limited by the number of bits, values above maximum or below minimum are
   *    clipped to the boundary
  */


  std::cout << "foo.item is " << foo.first_item << std::endl;
  for (int i=0; i<8; i++)
  {
    std::cout << "foo.array[" << i << "] is " << foo.array_item[i] << std::endl;
  }

}
