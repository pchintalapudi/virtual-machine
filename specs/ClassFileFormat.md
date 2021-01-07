# Class File Format

## Breakdown

1. Header
   1. 4-byte Magic number (0095F11E)
   1. 2-byte Implemented count
   1. 2-byte Method count
   1. 4-byte Instance field table offset
   1. 4-byte Static field table offset
   1. 4-byte Import table offset
   1. 4-byte Method table offset
   1. 4-byte String pool offset
1. Class References
   1. 4-byte String offsets
1. Instance Fields
   1. Fields
      1. 1-byte Metadata
      1. 3-byte string offset
1. Static Fields
   1. Fields
      1. 1-byte Metadata
      1. 3-byte string offset
1. Dynamic imports
    1. Imports
        1. 2-byte Metadata
        1. 2-byte class reference offset
        1. 4-byte string offset
1. Methods
   1. See /MethodStructure.md for method structure
      - Changes:
        1. Name is a 4-byte string offset prior to method bulk
        1. An additional 4-byte offset of 0 will be present unless this is a native method, in which case this will be an offset to the native method's name
1. String Pool
   1. String units
      1. 4-byte string length
      1. n-byte string characters

First string in string pool must be class name. The first 6 classes in the class reference table must be byte, short, int, float, long, and double, in that order. The seventh class (index 6) must have index 0 (i.e. index 6 indicates the currently-loading class), the eight class (index 7) indicates the superclass from which inheritance details are inherited from (the primary superclass), while all classes immediately following the eigth class up to the implemented count are implemented interfaces.
