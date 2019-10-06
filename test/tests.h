#include <iostream>
#include <iomanip>
#include <array>
#include "vstruct.h"

void check(bool chk)
{
  if(!chk)
//    throw -1;
    std::cout<<"BAD!" << std::endl;
}

namespace vstruct{

  template <class T, uint16_t Sz, int N>
  class TestArray
  {
    public:
    struct _TestStruct: public vstruct::VStruct
    {
      Array<bool, 1>  b0{*this, 8};
      Array<T, Sz>    x{*this, N};
      Array<bool, 1>  b1{*this, 8};
    };
    _TestStruct obj{};
    uint8_t _arr[2 + (((Sz * N) + 7) >> 3)] = {0};
    uint8_t* ptr=nullptr;
    T ref[N] = {0};
    T packed_max;
    T packed_min;
    T unpacked_max;
    T unpacked_min;
    
    TestArray()
    {
      
      if(std::is_signed<T>::value)
      {
        packed_max = ((1u) << (Sz - 1)) - 1;
        packed_min = ~packed_max;
        unpacked_max = ((T)-1) >> 1;
        unpacked_min = ~unpacked_max;
      }
      else
      {
        packed_max = ((1u) << Sz) - 1;
        packed_min = 0;
        unpacked_max = -1;
        unpacked_min = 0;
      }
      for(size_t i=0; i<sizeof(_arr); i++)
        _arr[i] = 0;
      for(size_t i=0; i<N; i++)
        ref[i] = 0;
      std::cout<<packed_min << ',' << packed_max << ',' << unpacked_min << ',' << unpacked_max <<std::endl;
    }


    void test_size()
    {
      uint16_t exp_bit_size = Sz * N + 8 + 8;
      uint16_t t1 = obj.bitSize();
      check( exp_bit_size == t1);

      uint16_t t2 = obj.byteSize();
      check( ((exp_bit_size + 7) >> 3) == t2);
    }  

    T _get_single_exp(T value)
    {
      if(value >= packed_max)
        value = packed_max;
      else if(value <= packed_min)
        value = packed_min;
      return value;
    }

    void _check_single_set(int index, T value)
    {
      
      int i;
      T test_val;
      T expected = _get_single_exp(value);
      std::cout << "Test [" << index << "] = "      
                <<std::hex<< std::setfill('0') << std::setw(2)<<expected << std::endl;

      for(i=0; i<8; i++)
      {
        T test_val = obj.b0[i];
        check(test_val == 0);
      }
      
      // print the original buffer
      std::cout<<"  ";
      for (i=0; i<sizeof(_arr); i++)
      {
        std::cout<<std::hex<< std::setfill('0') << std::setw(2)<<(int)_arr[i];
        if(i & 0x3 == 0x3)
        std::cout<<" ";
      }
      std::cout<<std::endl;

      // set the value
      obj.x[index] = value;

      // update reference
      ref[index] = expected;

      // check the value
      for (i=0; i<N; i++)
      {
        test_val = obj.x[i];
        check(ref[i] == test_val);
      }
      
      // print the buffer after changes
      std::cout<<"  ";
      for (i=0; i<sizeof(_arr); i++)
      {
        std::cout<<std::hex<< std::setfill('0') << std::setw(2)<<(int)_arr[i];
        if(i & 0x3 == 0x3)
          std::cout<<" ";
      }
      std::cout<<std::endl;

      for(i=0; i<8; i++)
      {
        T test_val = obj.b1[i];
        check(obj.b1[i] == 0);
      }
    }

    void _test(T value, bool odd)
    {
      for(int i = 0; i < N; i++)
      {
        if((bool)(i & 1u) && odd)
          _check_single_set(i, value);
        else if(!(bool)(i & 1u) && !odd)
          _check_single_set(i, value);
      }
    }
    void test_all_odd()
    {
      _test(1, true);
      _test(-1, true);
      _test(packed_max, true);
      _test(packed_min, true);
      _test(packed_max - 1, true);
      _test(packed_min - 1, true);
      if(Sz < sizeof(T) * 8)
      {
        _test(packed_max + 1, true);
        _test(packed_min + 1, true);
      }
      _test(unpacked_max, true);
      _test(unpacked_min, true);
      _test(unpacked_max - 1, true);
      _test(unpacked_min - 1, true);
      _test(0, true);
    }
    void test_all_even()
    {
      _test(1, false);
      _test(-1, false);
      _test(packed_max, false);
      _test(packed_min, false);
      _test(packed_max - 1, false);
      _test(packed_min - 1, false);
      if(Sz < sizeof(T) * 8)
      {
        _test(packed_max + 1, true);
        _test(packed_min + 1, true);
      }
      _test(unpacked_max, false);
      _test(unpacked_min, false);
      _test(unpacked_max - 1, false);
      _test(unpacked_min - 1, false);
      _test(0, false);
    }
    void run()
    {
      ptr = _arr;
      obj.setBuffer(ptr);
      test_size();
      test_all_odd();
      test_all_even();
    }
  };

}