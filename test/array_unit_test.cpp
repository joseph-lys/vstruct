#include <iostream>
// #include <iomanip>

#include "gtest/gtest.h"
#include "vstruct.h"
#include "test_functions.h"
//#include "test_helper.h"

template<typename T, uint16_t Sz>
class Object
{
public:
  T a_[Sz]={0};
};

namespace{
  using testing::Types;

  template <typename Targ, uint16_t Szarg, size_t Narg>
  struct TestArgs
  {
    typedef Targ T;
    enum : uint16_t { Sz = Szarg };
    enum : size_t { N = Narg };
  };

  typedef Types<
    TestArgs<bool, 1, 8>,
    TestArgs<uint8_t, 7, 8>,
      TestArgs<int8_t, 7, 8>,
    TestArgs<uint16_t, 7, 8>,
      TestArgs<uint16_t, 15, 8>,
    TestArgs<int16_t, 7, 8>,
      TestArgs<int16_t, 15, 8>,
    TestArgs<uint32_t, 7, 8>,
      TestArgs<uint32_t, 15, 8>,
      TestArgs<uint32_t, 23, 8>,
      TestArgs<uint32_t, 31, 8>,
    TestArgs<int32_t, 7, 8>,
      TestArgs<int32_t, 15, 8>,
      TestArgs<int32_t, 23, 8>,
      TestArgs<int32_t, 31, 8>
      > AllTestArgs;
  
  template <class TArgs>
  class ArrayTestSuite : public testing::Test
  {
  public:
    Object<typename TArgs::T, TArgs::Sz> obj;
    uint8_t b_[TArgs::N] = {0};
    ArrayTestSuite()
    {
      std::cout << "Test Class Created!" << std::endl;
    }
  };

  TYPED_TEST_SUITE_P(ArrayTestSuite);

  TYPED_TEST_P(ArrayTestSuite, MaximumPacked)
  {
    tfuncExpectEqual(this->obj.a_, this->b_,0);
  }

  REGISTER_TYPED_TEST_SUITE_P
  (
      ArrayTestSuite,
      MaximumPacked
  );

  INSTANTIATE_TYPED_TEST_SUITE_P
  (
      InstanceName,
      ArrayTestSuite,
      AllTestArgs
  );

}


// TEST(DummyTest, TypedTestsAreNotSupportedOnThisPlatform) {}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
