#include <iostream>
#include <iomanip>

#include "gtest/gtest.h"
#include "vstruct.h"
#include "test_suite_base.h"

namespace{

using testing::Types;

typedef Types<
  //TestArgs<bool, 1, 8>,  // bool is a special case, causing template issues
  TestArgs<uint8_t, 7, 7>,
    TestArgs<int8_t, 7, 7>,
    TestArgs<uint8_t, 7, 8>,
    TestArgs<int8_t, 7, 8>,
    TestArgs<uint8_t, 7, 9>,
    TestArgs<int8_t, 7, 9>,
  TestArgs<uint16_t, 7, 8>,
    TestArgs<uint16_t, 15, 8>,
  TestArgs<int16_t, 7, 8>,
    TestArgs<int16_t, 15, 8>,
  TestArgs<uint32_t, 7, 7>,
    TestArgs<uint32_t, 15, 7>,
    TestArgs<uint32_t, 23, 7>,
    TestArgs<uint32_t, 31, 7>,
    TestArgs<uint32_t, 7, 8>,
    TestArgs<uint32_t, 15, 8>,
    TestArgs<uint32_t, 23, 8>,
    TestArgs<uint32_t, 31, 8>,
    TestArgs<uint32_t, 7, 9>,
    TestArgs<uint32_t, 15, 9>,
    TestArgs<uint32_t, 23, 9>,
    TestArgs<uint32_t, 31, 9>,
  TestArgs<int32_t, 7, 7>,
    TestArgs<int32_t, 15, 7>,
    TestArgs<int32_t, 23, 7>,
    TestArgs<int32_t, 31, 7>,
    TestArgs<int32_t, 7, 8>,
    TestArgs<int32_t, 15, 8>,
    TestArgs<int32_t, 23, 8>,
    TestArgs<int32_t, 31, 8>,
    TestArgs<int32_t, 7, 9>,
    TestArgs<int32_t, 15, 9>,
    TestArgs<int32_t, 23, 9>,
    TestArgs<int32_t, 31, 9>
    > IntTestArgs;
typedef Types<TestArgs<int16_t, 15, 9>> IntSingleTestArgs;

template<typename T, uint16_t Sz, size_t N>
class DummyTestStruct
{
public:
  struct Property
  {
    T value_;
    Property():value_(0){}
    Property& operator=(const T& value)
    {
      if(std::is_signed<T>::value)
      {
        T max_val = ((1u << (Sz - 1)) - 1);
        T min_val = ~max_val;
        if(value > max_val)
          value_ = max_val;
        else if(value < min_val)
          value_ = min_val;
        else
          value_ = value;
      }
      else
      {
        T max_val = (1u << Sz) - 1;
        if(value > max_val)
          value_ = max_val;
        else
          value_ = value;
      }
    }

    operator T () const // getter
    {
      return value_;
    }

  };
  bool b0[8]{false};
  Property target[N]{};
  bool b1[8]{false};
};


  
  template <typename TArgs>
  class ArrayTestSuite : public TestSuiteBase <TArgs>
  {

  public:
    typename TArgs::T expected_[((TArgs::N * TArgs::Sz + 7) >> 3) + 2];
    DummyTestStruct<typename TArgs::T, TArgs::Sz, TArgs::N> vstruct_;
    constexpr size_t byteSize(){ return ((TArgs::N * TArgs::Sz + 7) >> 3) + 2; }
    ArrayTestSuite() : TestSuiteBase <TArgs>{byteSize()}, expected_{0} {}
    void doWrite(size_t index, typename TArgs::T value);
    void testWriteRead(size_t index, typename TArgs::T value);
    void testAllWriteRead(typename TArgs::T value);
    void testOddWriteRead(typename TArgs::T value);
    void testEvenWriteRead(typename TArgs::T value);
  };

  template <typename TArgs>
  void ArrayTestSuite<TArgs>::doWrite(size_t index, typename TArgs::T value)
  {
    std::stringstream ss;
    ss << "OP: [" << index << "] = "
       << std::hex << std::setfill('0') << std::setw(8) << (size_t)value
       << std::endl;
    ArrayTestSuite::dump_desc_op_ = ss.str();
    ArrayTestSuite::dump_before_op_ = "  Before: " + ArrayTestSuite::bufferDump();
    vstruct_.target[index] = value;
    ArrayTestSuite::expected_[index] = ArrayTestSuite::clipValue(value);  // expected behaviour is clipped to bit size
    ArrayTestSuite::dump_after_op_ =  "  After : " + ArrayTestSuite::bufferDump();
  }

  template <typename TArgs>
  void ArrayTestSuite<TArgs>::testWriteRead(size_t index, typename TArgs::T value)
  {
    doWrite(index, value);
    typename TArgs::T x= vstruct_.target[index];
    EXPECT_TRUE(x == expected_[index])
      << "IN " << ArrayTestSuite::dump_desc_op_ << std::endl
      << "  LHS[" << index << "]=" << x << ", RHS[" << index << "]=" << expected_[index] << std::endl
      << ArrayTestSuite::dump_before_op_
      << ArrayTestSuite::dump_after_op_;
  }

  template <typename TArgs>
  void ArrayTestSuite<TArgs>::testAllWriteRead(typename TArgs::T value)
  {
    for(size_t i=0; i<TArgs::N; i++)
    {
      testWriteRead(i, value);
    }
  }

  template <typename TArgs>
  void ArrayTestSuite<TArgs>::testOddWriteRead(typename TArgs::T value)
  {
    for(size_t i=0; i<TArgs::N; i++)
    {
      if(i & 1)
      {
         testWriteRead(i, value);
      }
    }
  }

  template <typename TArgs>
  void ArrayTestSuite<TArgs>::testEvenWriteRead(typename TArgs::T value)
  {
    for(size_t i=0; i<TArgs::N; i++)
    {
      if((i & 1) == 0)
      {
         testWriteRead(i, value);
      }
    }
  }



  TYPED_TEST_SUITE_P(ArrayTestSuite);

  TYPED_TEST_P(ArrayTestSuite, TestwareValidation)
  {
    if(std::is_signed<decltype(this->minPacked())>::value)
    {
      EXPECT_EQ( this->maxPacked() ^ this->minPacked(),(decltype(this->minUnpacked())) -1)
        << std::hex << "Packed Max " << (size_t)this->maxPacked() << ",  PackedMin " << (size_t)this->minPacked();
    }
    EXPECT_EQ( this->maxUnpacked() ^ this->minUnpacked(), (decltype(this->minUnpacked())) -1)
      << std::hex << "Unpacked Max " << (size_t)this->maxUnpacked() << ",  UnpackedMin " << (size_t)this->minUnpacked();
  }

  TYPED_TEST_P(ArrayTestSuite, MaximumPacked)
  {
    this->testOddWriteRead(this->maxPacked());
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->maxPacked());
    this->testEvenWriteRead(0);
  }
  TYPED_TEST_P(ArrayTestSuite, MinimumPacked)
  {
    this->testOddWriteRead(this->minPacked());
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->minPacked());
    this->testEvenWriteRead(0);
  }
  TYPED_TEST_P(ArrayTestSuite, MaximumUnpacked)
  {
    this->testOddWriteRead(this->maxUnpacked());
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->maxUnpacked());
    this->testEvenWriteRead(0);
  }
  TYPED_TEST_P(ArrayTestSuite, MinimumUnpacked)
  {
    this->testOddWriteRead(this->minUnpacked());
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->minUnpacked());
    this->testEvenWriteRead(0);
  }

  TYPED_TEST_P(ArrayTestSuite, NearMaximumPacked)
  {
    this->testOddWriteRead(this->maxPacked() - 1);
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->maxPacked() - 1);
    this->testEvenWriteRead(0);
  }
  TYPED_TEST_P(ArrayTestSuite, NearMinimumPacked)
  {
    this->testOddWriteRead(this->minPacked() + 1);
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->minPacked() + 1);
    this->testEvenWriteRead(0);
  }
  TYPED_TEST_P(ArrayTestSuite, NearMaximumUnpacked)
  {
    this->testOddWriteRead(this->maxUnpacked() - 1);
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->maxUnpacked() - 1);
    this->testEvenWriteRead(0);
  }
  TYPED_TEST_P(ArrayTestSuite, NearMinimumUnpacked)
  {
    this->testOddWriteRead(this->minUnpacked() + 1);
    this->testOddWriteRead(0);
    this->testEvenWriteRead(this->minUnpacked() + 1);
    this->testEvenWriteRead(0);
  }


  REGISTER_TYPED_TEST_SUITE_P
  (
      ArrayTestSuite,
      TestwareValidation,
      MaximumPacked,
      MinimumPacked,
      MaximumUnpacked,
      MinimumUnpacked,
      NearMaximumPacked,
      NearMinimumPacked,
      NearMaximumUnpacked,
      NearMinimumUnpacked
  );

  INSTANTIATE_TYPED_TEST_SUITE_P
  (
      InstanceName,
      ArrayTestSuite,
      IntTestArgs
      // IntSingleTestArgs  // run only 1
  );

}


// TEST(DummyTest, TypedTestsAreNotSupportedOnThisPlatform) {}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();  // need to return this for correct return value
}
