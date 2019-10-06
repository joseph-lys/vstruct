#include <iostream>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>
#include "gtest/gtest.h"

static uint8_t* g_test_buffer = nullptr;
static size_t g_test_buffer_size = 0;
static std::string g_before_op_dump;
static std::string g_after_op_dump;



std::string tfuncBufferDump()
{
  std::string s = "Test Buffer Not Initialized";
  if(g_test_buffer != nullptr)
  {
    std::ostringstream ss;
    for(size_t i=0; i < g_test_buffer_size; i++)
    {
      ss<< std::hex << std::setfill('0') << std::setw(2)<< (uint16_t)g_test_buffer[i];
      if(i & 3)
      {
        ss << " ";
      }
    }
    ss << std::endl;
    s = ss.str();
  }
  return s;
}


void tfuncSetup(uint8_t* allocated_buffer, size_t allocated_size)
{
  g_test_buffer = allocated_buffer;
  g_test_buffer_size = allocated_size;
  g_before_op_dump = tfuncBufferDump();
  g_after_op_dump = tfuncBufferDump();
}


template<typename T1, typename T2>
void tfuncExpectEqual(T1 LHS, T2 RHS, size_t N)
{
  bool check;
  for(size_t i=0; i<N; i++)
  {
    EXPECT_TRUE(LHS[i] == RHS[i])
        << "NEQ! = LHS[" << i << "]=" << LHS[i] << ", RHS[" << i << "]=" << RHS[i] << std::endl
        << g_before_op_dump << std::endl
        << g_after_op_dump << std::endl;
  }
}
