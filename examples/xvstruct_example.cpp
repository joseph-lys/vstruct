#include <iostream>
#include "xvstruct.h"

/*
 * Some example code on how to use vstruct.
 *
 * How to use
 * 1. Define a struct using vstruct::VStruct
 *
 *
*/

struct Foo : xvstruct::XVStruct
{
  /*
   * The decltype is required as auto not supported :-(
   * you can use a macro to make things nicer
   */

  /*
   * the Root item is attached to the internal buffer pointer pbuf_
   */
  xvstruct::RootItem root_{pbuf_};

  /* template parameters for LEItem
   * are <[item type], [Number of bits]>
   *
   * the Item is attached to starting position of the root item
   */

  decltype(xvstruct::addLEItem<int, 8>(root_)) first_item = xvstruct::addLEItem<int, 8>(root_);


  /* template parameters for LEArray
   * are <[item type], [Number of bits], [Array Size]>
   *
   * the Array is attached to buffer position right after the first_item
   */
  decltype(xvstruct::addLEArray<int, 7, 5>(first_item)) array_item = xvstruct::addLEArray<int, 7, 5>(first_item);


  /* template parameters for AlignPad
   * are <[Alignment Bits]>
   *
   * the padding is attached to buffer position right after the array_item
   */
  decltype(xvstruct::addAlignPad<16>(array_item)) padding = xvstruct::addAlignPad<16>(array_item);


  /* BoolItem does not require any parameter arguments
   *
   * the bit item is attached to buffer position right after the padding
   */
  decltype(xvstruct::addBoolItem(padding)) bit_value = xvstruct::addBoolItem(padding);


  /* template parameters for BoolArray
   * are <[Array Size]>
   *
   * the bit item is attached to buffer position right after the padding
   */
  decltype(xvstruct::addBoolArray<3>(bit_value)) bit_array = xvstruct::addBoolArray<3>(bit_value);

  /* floats are not guaranteed to work.
   * as currently there is no checking of cpu byte order at compile time
   */
  decltype(xvstruct::addLEArray<float, 32, 5>(bit_array)) float_array = xvstruct::addLEArray<float, 32, 5>(bit_array);

  /* Helper Macro example
   *
   */
#define HELPER_MACRO(N,P,F,...)   decltype(F<__VA_ARGS__>(P)) N = F<__VA_ARGS__>(P)
  HELPER_MACRO(macro_array1, float_array, xvstruct::addLEArray, unsigned int, 5, 3);
  HELPER_MACRO(macro_array2, macro_array1, xvstruct::addLEArray, int, 13, 5);
#undef HELPER_MACRO


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

  foo.first_item = 10;

  foo.array_item[0] = 123;
  foo.array_item[1] = -123;
  foo.array_item[2] = 1000000;
  foo.array_item[3] = -1000000;
  foo.array_item[4] = 100;

  foo.float_array[0] = -123.123;
  foo.float_array[1] = 123.123;
  foo.float_array[2] = 255.0;
  foo.float_array[3] = -255.0;

  /* Note:
   * 1. The array type has no boundary checking. MAKE Sure not to go over the assigned size!
   * 2. Values are limited by the number of bits, values above maximum or below minimum are
   *    clipped to the boundary
  */


  std::cout << "foo.item is " << foo.first_item << std::endl;
  for (int i=0; i<8; i++)
  {
    auto x = foo.array_item[i];
    std::cout << "foo.array[" << i << "] is " << x << std::endl;
  }

  for (int i=0; i<5; i++)
  {
    float x = foo.float_array[i];
    std::cout <<  "foo.float_array[" << i << "] is " << std::to_string(x) << std::endl;
  }


  foo.macro_array1[1] = 20;
  std::cout << foo.macro_array1[1]<<std::endl;

}
