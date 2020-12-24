# Method Structure

---

## Outline

1. Stack allocations - 28 bytes
   1. Total size - 4 bytes
   1. Double offset - 4 bytes
   1. Long offset - 4 bytes
   1. Float offset - 4 bytes
   1. Int offset - 4 bytes
   1. Short offset - 4 bytes
   1. Byte offset - 4 bytes
1. Bytecode offset - 4 bytes
1. Additional metadata - 4 bytes
1. Bytecode size - 2 bytes
1. Arguments
   1. Count - 1 byte
   1. Types - Count / 2 bytes
1. Alignment - 0s up to 8-byte aligned

Method header varies in size between 40-112 bytes
