# Class File Layout

0. Section table
   1. 64 bit offsets
      1. Class references
      2. Method references
      3. Static variables
      4. Instance variables
      5. Bytecode
      6. Length-string pool

1. Class reference section
   1. 32 bit Class count (indexes 6 reserved for this class, 0-5 are reserved for char, short, int, long, float, and double respectively)
   2. 32 bit Implemented/Extended class count
   3. Class references
      1. Length-string 64 bit base offset

2. Method reference section
   1. 32 bit method count
   2. 32 bit self-static count
   3. Method references
      1. 32 bit class reference
      2. 32 bit zeros
      3. Length-string 64 bit base offset

3. Static variables section
   1. 32 bit count
   2. 32 bit handle count
   3. Static reference
      1. 32 bit host class reference
      2. 32 bit type class reference
      3. Length-string 64 bit base offset

4. Instance variables section
   1. 32 bit count
   2. 32 bit handle count
   3. Instance reference
      1. 32 bit host class reference
      2. 32 bit type class reference
      3. Length-string 64 bit base offset

5. Bytecode section
   1. 64 bit bytecodes size
   2. Compiled methods
      1. 64 bit method bytecode length
      2. \[\[compiled method\]\]

6. Null-terminated string pool section
   1. 64 bit pool size
   2. Pool
      1. Length-string
