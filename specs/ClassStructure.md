# Loaded Class Structure

---

## Class Outline

1. Common Metadata (sizes, offsets, protection bounds, etc.)
1. Static Memory Region (dynamically changeable)
1. Virtual Method Table
1. Descriptor Table
1. Reflection Table (lexicographically sorted)
1. Method Bytecodes
1. String Pool (ordered by file layout, suggested presorted) [may be interned for memory savings]

---

## Detailed Descriptions

1. Common Metadata - 96 bytes
   1. Superclass pointer - 8 bytes
   1. Static Variable Metadata - 4 x 7 = 28 bytes
      1. Total Size
      1. Double Offset
      1. Long Offset
      1. Float Offset
      1. Int Offset
      1. Short Offset
      1. Byte Offset
   1. Instance Variable Metadata - 4 x 7 = 28 bytes
      1. Total Size
      1. Double Offset
      1. Long Offset
      1. Float Offset
      1. Int Offset
      1. Short Offset
      1. Byte Offset
   1. VMT Offset - 4 bytes
   1. Class Descriptor Table Offset - 4 bytes
   1. Field Descriptor Table Offset - 4 bytes
   1. Reflection Strings Offset - 4 bytes
   1. Method Bytecodes Offset - 4 bytes
   1. String Pool Offset - 4 bytes
   1. Total class size - 4 bytes
   1. Class Index - 4 bytes
1. Static Memory Region
   1. pointers
   1. doubles
   1. longs
   1. floats
   1. ints
   1. shorts
   1. bytes
      Padded to 8 byte alignment
1. Virtual Method Table
   1. Method pointers - 8 bytes each
1. Descriptor Table
   1. Class Descriptors
      1. 4-byte string offset / class index
   1. Field Descriptors
      1. 4-byte string offset / field index
      1. 4-byte class offset / class index
1. Reflection Table
   1. 4-byte string offset
   1. 3-byte index + 1-byte metadata
1. Method Bytecodes
   1. Methods (see /MethodStructure.md)
1. String Pool
   1. String pointers - 8 bytes each
