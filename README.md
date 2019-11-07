# vstruct
virtual struct for bit packing


## What is this about?
For cross platfrom "safe" version of C struct. 
No worries of counting alignment padding and endians.
Arrays of bit fields!


## Who should use this?
### vstruct
You want to prototype stuff, you have excess memory and cpu capacity, your data structure is still changing and you don't want to keep reworking the bit access. 


## Features
* bool type
* Signed and unsigned interger types up to 64bit sizes.
* Arrays of the above types

> float and double might work. (assuming 32 bit float, 64 bit double, same storage order as int)


## Requirements
C++11.
Testsuite requires Google Test.
Python 3


## Notes
> Values are exceeding maximum or below minimum bit field capacity are clipped.
> Currently only support Little Endian byte order



