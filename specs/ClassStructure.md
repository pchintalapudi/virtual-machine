# Loaded Class Structure

---

## Class Outline

1. Common Metadata (sizes, offsets, protection bounds, etc.)
1. Static Memory Region (dynamically changeable)
1. Virtual Method Table
1. Reflection Strings (lexicographically sorted)
1. Method Bytecodes
1. String Pool (ordered by file layout, suggested presorted) [may be interned for memory savings]

---

## Detailed Descriptions

1. Common Metadata
    1. Superclass pointer - 8 bytes
    1. Overall object size - 4 bytes
    1. Instance variable metadata - 14 bytes
        1. Pointer count - 2 bytes
        1. double count - 2 bytes
        1. long count - 2 bytes
        1. float count - 2 bytes
        1. int count - 2 bytes
        1. short count - 2 bytes
        1. byte count - 2 bytes
    1. Static variable metadata - 14 bytes
        1. Pointer count
        1. double count
        1. long count
        1. float count
        1. int count
        1. short count
        1. byte count
    1. VMT Metadata - 4 bytes
        1. Virtual method count
    1. Reflection String Metadata - 4 bytes
        1. String count
    1. Method Bytecode Metadata - 4 bytes
        1. Total bytecode size
    1. String Pool Metadata - 4 bytes
        1. String count
    Total size 56 bytes
2. Static Memory Region
    1. Array pointers
    1. Object pointers
    1. doubles
    1. longs
    1. floats
    1. ints
    1. shorts
    1. bytes
    Padded to 8 byte alignment
3. Virtual Method Table
    1. Method pointers - 8 bytes each
4. Reflection Strings
    1. Unit
        1. Metadata - high byte of 4 bytes
        1. Offset - low 3 bytes of 4 bytes
        1. String length - 4 bytes
        1. String - n bytes
        Padded to 8 byte alignment
5. Method Bytecodes
    1. Methods (see /MethodStructure.md)
6. String Pool
    1. String pointers - 8 bytes each
