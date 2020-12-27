# Method Structure

---

## Outline

1. Stack allocations - 10 bytes
   1. Total size - 2 bytes
   1. Double offset - 2 bytes
   1. Long offset - 2 bytes
   1. Float offset - 2 bytes
   1. Int offset - 2 bytes
1. Additional metadata - 2 bytes
1. Bytecode offset - 2 bytes
1. Bytecode size - 2 bytes
1. Method name - 8 bytes
1. Arguments
   1. Count - 1 byte
   1. Types - Count / 2 bytes
1. Alignment - 0s up to 8-byte aligned
1. Method instructions

Method header varies in size between 32-104 bytes
