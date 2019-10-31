#include <iostream>
#include "vstruct.h"

/*
 * Some example code on how to use vstruct.
 *
 * How to use
 * 1. Define a struct using vstruct::VStruct
 * 2. Populate with vstruct::Item or vstruct::Array templates
 * 3. vstruct::Item and vstruct::Array has to be initialized to the instance of vstruct::Vstruct
 *
*/

struct Foo final : public vstruct::VStruct {
  /* template parameters for Item
   * are <[item type], [Number of bits]>
   *
   * the Item is initialzed to the Foo vstruct instance by
   * calling the initializer {*this}
   */
  typename vstruct::LEItem<vstruct::Root, unsigned int, 5>::type item{*this};

  /* template parameters for Array
   * are <[item type], [Number of bits]>
   *
   * the Item is initialzed to the Foo vstruct instance by
   * calling the initializer {*this, N}
   * The additional N argument in the array is the number of array
   * elments. in this example an array of 5 is used
   */
  typename vstruct::LEArray<decltype(item), long int, 3, 5>::type array{*this};

  /* template parameters for AlignPad
   * are <[AlignByte]>
   *
   * the Item is initialzed to the Foo vstruct instance by
   * calling the initializer {*this}
   */
  typename vstruct::AlignPad<decltype(array), 2>::type pad{*this};


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
  const int asize = foo.array.cummulativeByteSize();
  uint8_t some_memory[foo.array.cummulativeByteSize()]{0};
  foo.setBuffer(some_memory);
  std::cout << "foo requires " << foo.array.cummulativeByteSize() << " Bytes" << std::endl << std::endl;


  //std::cout << "foo.item is " << foo.item << std::endl;
  for (int i=0; i<5; i++)
  {
    std::cout << "foo.array[" << i << "] is " << foo.array[i] << std::endl;
  }

  /* use the members as you would a normal interger struct member.
   * here are some assignments
  */

  foo.item = 1;
  foo.array[1] = 123;
  foo.array[2] = -123;
  foo.array[3] = 1000000;
  foo.array[4] = -1000000;

  /* Note:
   * 1. The array type has no boundary checking. MAKE Sure not to go over the assigned size!
   * 2. Values are limited by the number of bits, values above maximum or below minimum are
   *    clipped to the boundary
  */

  long int x;
  std::cout << "foo.item is " << foo.item << std::endl;
  x = foo.array[0];
  std::cout << "foo.array[0] is " << x << std::endl;
  x = foo.array[1];
  std::cout << "foo.array[1] is " << x << std::endl;
  x = foo.array[2];
  std::cout << "foo.array[2] is " << x << std::endl;
  x = foo.array[3];
  std::cout << "foo.array[3] is " << x << std::endl;
  x = foo.array[4];
  std::cout << "foo.array[4] is " << x << std::endl;


}
