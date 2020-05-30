# Bytecode definition

## Global Resource Limits

- Methods must be 2^16 instructions or less
  - Required by branching instructions, otherwise limited to 24 bits by pushhdr
- No object may exceed 2^32 bytes in size
  - Required by compressed object layout with 32 bits occupied by counting handles
- Runs on 64 bit system
  - Required by assumption that sizeof(T*) == sizeof(64-bit type)
  - Also performance assumptions are based on this
- Runs on little-endian system
  - Required by instructions that implicitly cast (e.g. read a short from a long's offset)

## Instruction Coding - 64 bit

- Standard

  ```
  |---------------|---------------|---------------|-------|-------|
  |               |               |               |       |       |
  |      src2     |      src1     |      dest     |typeinf| opcode|
  |---------------|---------------|---------------|-------|-------|
  ```

- LUI

  ```
  |-------------------------------|---------------|-------|-------|
  |                               |               |       |       |
  |              uimm             |      dest     |typeinf| opcode|
  |-------------------------------|---------------|-------|-------|
  ```

- ARGS

  ```
  |---------------|---------------|---------------|---------------|
  |               |               |               |               |
  |      arg2     |      arg1     |      arg0     |   argtypes    |
  |---------------|---------------|---------------|---------------|
  ```

## Primitive operations - 0b0110

1. add - 0x69
2. sub - 0x68
3. mul - 0x67
4. div - 0x66
5. addi - 0x65
6. muli - 0x64
7. divi - 0x63
8. neg - 0x62
9. cast - 0x61
10. lui - 0x60

## Integer operations - 0b0101

1. and - 0x5b
2. or - 0x5a
3. xor - 0x59
4. sll - 0x58
5. srl - 0x57
6. sra - 0x56
7. andi - 0x55
8. ori - 0x54
9. xori - 0x53
10. slli - 0x52
11. srli - 0x51
12. srai - 0x50

## Control Flow - 0b0100

1. bge - 0x4d
2. blt - 0x4c
3. ble - 0x4b
4. bgt - 0x4a
5. beq - 0x49
6. bneq - 0x48
7. bgei - 0x47
8. blti - 0x46
9. blei - 0x45
10. bgti - 0x44
11. beqi - 0x43
12. bneqi - 0x42
13. ba - 0x41
14. jump - 0x40

## Constants - 0b0011

1. lconst - 0x31
2. sconst - 0x30

## Objects - 0b0010

1. load - 0x25
2. store - 0x24
3. new - 0x23
4. classof - 0x22
5. instanceof - 0x21
6. lclass - 0x20

## Exceptions - 0b0001

1. throw - 0x12
2. pushhdr - 0x11
3. pophdr - 0x10

## Methods - 0b0000

1. invoke - 0x04
2. args - undefined
3. ret - 0x02
4. vd - 0x01
5. nop - 0x00

### Calling Convention

#### Caller Convention

1. Invoke is called on a method, with a packed 8 bit return value type, 16 bit method location offset, a 16 bit return value offset, and 16 bit argument count.
2. Following this, a packed version of arguments follow, where each 64 bit word is comprised of the next 3 arguments in order, followed by a packed 16 bit integer carrying the type information of each argument.

#### Callee Convention

1. The return opcode must have 8 bits of type information, as well as a 16 bit offset of the return value's location. All other bits must be 0.
2. If returning void, the opcode must be 1 and all other bits must be 0.
