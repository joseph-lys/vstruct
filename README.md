# vstruct
virtual struct for bit packing

## What is this about?
For cross platfrom "safe" version of C struct. This uses **a lot more cpu and memory** than hand-writen bit access.

## Who should use this?
You want to prototype stuff, you have excess memory and cpu capacity, your data structure is still changing and you don't want to keep reworking the bit access. 

## Features
* bool type
* Signed and unsigned interger types up to 32bit sizes. (64 bits should work, but not in testsuites)
* Arrays of the above types

## Requirements
C++11.
Testsuite requires Google Test.

## Notes

> Values are exceeding maximum or below minimum bit field capacity are clipped.
> Data is stored in Little Endian byte order
