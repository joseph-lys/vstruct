#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>
#include "gtest/gtest.h"

template <typename Targ, uint16_t Szarg, size_t Narg>
struct TestArgs
{
  typedef Targ T;
  enum : uint16_t { Sz = Szarg };
  enum : size_t { N = Narg };
};

template <typename TArgs>
class TestSuiteBase : public testing::Test
{
private:
  TestSuiteBase() = delete;
protected:
  uint8_t* test_buffer_ = nullptr;
  const size_t test_buffer_size_ = 0;
  std::string dump_before_op_;
  std::string dump_after_op_;
  std::string dump_desc_op_;
public:
  TestSuiteBase<TArgs>(size_t allocated_size);
  std::string bufferDump();
  typename TArgs::T clipValue(typename TArgs::T value);
  typename TArgs::T maxPacked();
  typename TArgs::T minPacked();
  typename TArgs::T maxUnpacked();
  typename TArgs::T minUnpacked();
};


template <typename TArgs>
std::string TestSuiteBase<TArgs>::bufferDump()
{
  std::string s = "Test Buffer Not Initialized";
  if(test_buffer_ != nullptr)
  {
    std::ostringstream ss;
    for(size_t i=0; i < test_buffer_size_; i++)
    {
      ss<< std::hex << std::setfill('0') << std::setw(2)<< (uint16_t)test_buffer_[i];
      if(i & 3 == 3)
      {
        ss << " ";
      }
    }
    ss << std::endl;
    s = ss.str();
  }
  return s;
}

template <typename TArgs>
TestSuiteBase<TArgs>::TestSuiteBase(size_t allocated_size)
  :test_buffer_size_(allocated_size)
{
  test_buffer_ = new uint8_t[allocated_size]{0};
  dump_before_op_ = bufferDump();
  dump_after_op_ = bufferDump();
  dump_desc_op_ = "";
}


template <typename TArgs>
typename TArgs::T TestSuiteBase<TArgs>::maxPacked()
{
  typename std::make_unsigned<typename TArgs::T>::type x;
  if(std::is_signed<typename TArgs::T>::value)
    x = 1u << (TArgs::Sz - 1);
  else
    x = 1u << TArgs::Sz;
  x -= 1;
  return (typename TArgs::T) x;
}

template <typename TArgs>
typename TArgs::T TestSuiteBase<TArgs>::minPacked()
{
  typename TArgs::T x;
  if(std::is_signed<typename TArgs::T>::value)
    x = ~maxPacked();
  else
    x = 0;
}

template <typename TArgs>
typename TArgs::T TestSuiteBase<TArgs>::maxUnpacked()
{
  typename TArgs::T x;
  if(std::is_signed<typename TArgs::T>::value)
    x = ((typename std::make_unsigned<typename TArgs::T>::type) -1 ) >> 1;
  else
    x = -1;
  return x;
}

template <typename TArgs>
typename TArgs::T TestSuiteBase<TArgs>::minUnpacked()
{
  typename TArgs::T x;
  if(std::is_signed<typename TArgs::T>::value)
    x = ~(typename std::make_unsigned<typename TArgs::T>::type)maxUnpacked();
  else
    x = 0;
  return x;
}

template <typename TArgs>
typename TArgs::T TestSuiteBase<TArgs>::clipValue(typename TArgs::T value)
{
  if(std::is_signed<typename TArgs::T>::value)
  { // clip to most negative
    if(value < minPacked())
      value = minPacked();
    else if (value > maxPacked())
      value = maxPacked();
  }
  else if(value > maxPacked())
  {
    value = maxPacked();
  }
  return value;
}
