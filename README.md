# vstruct
virtual struct for bit packing

## What is this about?
For cross platfrom "safe" version of C struct. 

vstruct uses **a lot more cpu and memory** than hand-writen bit access.
xvstruct will do reasonably well with minimal storage. Array access will depend of compiler ability to optimize the indexing. the syntax is pretty heavy though.


## Who should use this?
### vstruct
You want to prototype stuff, you have excess memory and cpu capacity, your data structure is still changing and you don't want to keep reworking the bit access. 

### xvstruct
You want to save memory space. But still too lazy to hand code everything.


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


### cpplint config
'''python -m cpplint --linelength=120 --root=include include/*.h include/*/*.h
