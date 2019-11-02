""" example1.py

copyright Joseph Lee Yuan Sheng 2019

"""
from vstruct import BoolItem, AlignPad, LEItem, LEArray, Type, VStruct


class Example1(VStruct):
    """ Example1

    this example struct is part of the testsuite
    """
    pad0 = AlignPad(4)
    # comment for b0
    b0 = BoolItem()  # Some comment
    b1 = BoolItem()
    b2 = BoolItem()
    pad1 = AlignPad(2)
    pad2 = AlignPad(1)  # this should align to same bit as pad1

    x0 = LEItem(Type.uint8_t, bit_size=2)
    x1 = LEItem(Type.int8_t, bit_size=3)
    x2 = LEItem(Type.uint16_t, bit_size=14)
    x3 = LEItem(Type.int16_t, bit_size=15)
    x4 = LEItem(Type.uint32_t, bit_size=26)
    x5 = LEItem(Type.int32_t, bit_size=27)
    x6 = LEItem(Type.uint64_t, bit_size=58)
    x7 = LEItem(Type.int64_t, bit_size=59)

    arr0 = LEArray(Type.uint8_t, bit_size=4, array_size=3)
    arr1 = LEArray(Type.int16_t, bit_size=11, array_size=11)
    arr2 = LEArray(Type.int16_t, bit_size=16, array_size=11)

    pad3 = AlignPad(2)

    flt = LEItem(Type.float)
    dbl = LEItem(Type.double)

    arr_flt = LEArray(Type.float, bit_size=32, array_size=4)
    arr_dbl = LEArray(Type.double, bit_size=64, array_size=4)

    pad4 = AlignPad(4)  # Pad to 4 bytes


