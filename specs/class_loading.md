# Class Loading

1. Commit memory
   * Need to determine size of memory to commit
   * What contributes to memory?
     1. Metadata
     2. Class references \* sizeof(char\*)
     3. Method references \* sizeof(char\*)
     4. Static references \* sizeof(char\*)
     5. Instance references \* sizeof(char\*)
     6. Bytecode size
     7. String pool size
     8. (Method + static + instance references) \* sizeof(std::uint32_t) + alignment
   * For the moment let's go for broke and just commit worst case memory (string pool size + bytecode size + reference count \* sizeof(char\*) + reference count \* sizeof(std::uint32_t) \* 2 + metadata size)
2. Init meta data
3. Load string pool with associated offsets
4. Load bytecodes with offsets
5. Inject class, method, static, and instance references

* Self-references are prewired to not require dynamic lookup
