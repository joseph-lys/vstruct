#include <iostream>
#include <iomanip>

#include "gtest/gtest.h"
#include "../include/vstruct/accessmethods.h"
#include "../include/vstruct/accessmethods.ipp"
#include "test_suite_base.h"

namespace{
using testing::Types;

typedef Types<
// uint8_t
TestArgs<uint8_t, 2,1>,
TestArgs<uint8_t, 3,1>,
TestArgs<uint8_t, 4,1>,
TestArgs<uint8_t, 5,1>,
TestArgs<uint8_t, 6,1>,
TestArgs<uint8_t, 7,1>,
TestArgs<uint8_t, 8,1>,

// uint16_t
TestArgs<uint16_t, 2,1>,
TestArgs<uint16_t, 3,1>,
TestArgs<uint16_t, 4,1>,
TestArgs<uint16_t, 7,1>,
TestArgs<uint16_t, 8,1>,
TestArgs<uint16_t, 9,1>,
TestArgs<uint16_t, 14,1>,
TestArgs<uint16_t, 15,1>,
TestArgs<uint16_t, 16,1>,


// uint32_t
TestArgs<uint32_t, 2,1>,
TestArgs<uint32_t, 3,1>,
TestArgs<uint32_t, 4,1>,
TestArgs<uint32_t, 23,1>,
TestArgs<uint32_t, 24,1>,
TestArgs<uint32_t, 25,1>,
TestArgs<uint32_t, 30,1>,
TestArgs<uint32_t, 31,1>,
TestArgs<uint32_t, 32,1>

> AllTestArgs;


template <typename TArgs>
class TestSuite: public TestSuiteBase <TArgs>
{
public:
  TestSuite() : TestSuiteBase <TArgs>{ 128 }{}
  enum
  {
    Sz = TArgs::Sz,
    N = TArgs::N
  };
  typedef typename TArgs::T T;
  typedef typename std::conditional<std::is_signed<T>::value, typename std::make_signed<T>::type, T>::type packedT;

  packedT pack(T val){ return vstruct::_internals::Clip<T, Sz>::packSign(val);}
  T unpack(packedT val){ return vstruct::_internals::Clip<T, Sz>::unpackSign(val);}

  bool test_write_max_bits(uint16_t offset)
  {
    volatile uint16_t size = Sz;

    uint8_t buffer[8] = {0};
    T val = (T)TestSuite::maxPacked();
    uint64_t actual = 0;
    uint64_t expected = ((uint64_t)val) << offset;
    vstruct::_internals::RawIF<packedT, Sz>::setLE(buffer, offset, val);

    for(int i=0; i < 8; i++)
    {
      actual <<= 8;
      actual |= buffer[7 - i];
    }

    if(actual != expected)
    {
      uint8_t dump[8] = {0};
      vstruct::_internals::RawIF<packedT, Sz>::setLE(dump, offset, val);
    }

    EXPECT_EQ(actual, expected);
  }
  bool test_read_max_bits(uint16_t offset)
  {
    volatile uint16_t size = Sz;
    uint8_t buffer[8] = {0};
    uint64_t expected = (uint64_t)TestSuite::maxPacked();

    vstruct::_internals::RawIF<packedT, Sz>::setLE(buffer, offset, expected);

    uint64_t temp = expected << offset;
    for(int i=0; i < 8; i++)
    {
      buffer[i] = temp & 0xff;
      temp >>= 8;
    }
    uint64_t actual = vstruct::_internals::RawIF<packedT, Sz>::getLE(buffer, offset);
    if(actual != expected)
    {
      uint64_t dump = vstruct::_internals::RawIF<packedT, Sz>::getLE(buffer, offset);
    }
    EXPECT_EQ(actual, expected);
  }
  void test_read_write(T value)
  {
    value = TestSuite::clipValue(value);
    volatile uint16_t size = Sz;
    for (uint16_t offset=0; offset < 17; offset++)
    {
      uint8_t buffer[8]={0};

      uint64_t expected = value;

      vstruct::_internals::RawIF<packedT, Sz>::setLE(buffer, offset, value);
      uint64_t actual = vstruct::_internals::RawIF<packedT, Sz>::getLE(buffer, offset);

      EXPECT_EQ(actual, expected);


      if(actual != expected)
      {
        uint8_t dump_write[8] = {0};
        vstruct::_internals::RawIF<packedT, Sz>::setLE(dump_write, offset, value);
        uint64_t dump_read = vstruct::_internals::RawIF<packedT, Sz>::getLE(buffer, offset);
      }
    }

  }
};

 TYPED_TEST_SUITE_P(TestSuite);

 TYPED_TEST_P(TestSuite, TestWriteMax)
 {
   this->test_write_max_bits(0);
   this->test_write_max_bits(1);
   this->test_write_max_bits(2);
   this->test_write_max_bits(3);
   this->test_write_max_bits(4);
   this->test_write_max_bits(5);
   this->test_write_max_bits(6);
   this->test_write_max_bits(7);
   this->test_write_max_bits(8);
   this->test_write_max_bits(9);
   this->test_write_max_bits(10);
   this->test_write_max_bits(11);
   this->test_write_max_bits(12);
 }

 TYPED_TEST_P(TestSuite, TestReadMax)
 {
   this->test_read_max_bits(0);
   this->test_read_max_bits(1);
   this->test_read_max_bits(2);
   this->test_read_max_bits(3);
   this->test_read_max_bits(4);
   this->test_read_max_bits(5);
   this->test_read_max_bits(6);
   this->test_read_max_bits(7);
   this->test_read_max_bits(8);
   this->test_read_max_bits(9);
   this->test_read_max_bits(10);
   this->test_read_max_bits(11);
   this->test_read_max_bits(12);
 }
 TYPED_TEST_P(TestSuite, TestReadWriteSome)
 {
   this->test_read_write(0xffffffff);
   this->test_read_write(0xf0f0f0f0);
   this->test_read_write(0x80808080);
   this->test_read_write(0x10101010);
   this->test_read_write(0x12121212);
   this->test_read_write(0x12345678);
   this->test_read_write(0x87654321);
 }



 REGISTER_TYPED_TEST_SUITE_P
 (
     TestSuite,
     TestWriteMax,
     TestReadMax,
     TestReadWriteSome
 );

 INSTANTIATE_TYPED_TEST_SUITE_P
 (
     InstanceName,
     TestSuite,
     AllTestArgs,       // run all
     //IntMinimalTestArgs  // run only 1
 );
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();  // need to return this for correct return value
}
