# Bytecode Instructions Available in the Virtual Machine

## Instruction Structure

---

* 64-bit standard width
* LSB to right

### 2-input Arithmetic Operation

```text
-------------------------------------------------------------------------
|        |        |                 |                 |                 |
| itype  |   0    |      src2       |      src1       |      dest       |
|        |        |                 |                 |                 |
-------------------------------------------------------------------------
```

### 2-input Arithmetic Immediate Operation

```text
-------------------------------------------------------------------------
|        |                          |                 |                 |
| itype  |          imm24           |      src1       |      dest       |
|        |                          |                 |                 |
-------------------------------------------------------------------------
```

### 1-input Cast Operation

```text
-------------------------------------------------------------------------
|        |                          |                 |                 |
| itype  |            0             |      src1       |      dest       |
|        |                          |                 |                 |
-------------------------------------------------------------------------
```

### 2-input Branch Operation

```text
-------------------------------------------------------------------------
|        |        |                 |                 |                 |
| itype  |   0    |      src2       |      src1       |     target      |
|        |        |                 |                 |                 |
-------------------------------------------------------------------------
```

### 2-input Branch Immediate Operation

```text
-------------------------------------------------------------------------
|        |                          |                 |                 |
| itype  |          imm24           |      src1       |     target      |
|        |                          |                 |                 |
-------------------------------------------------------------------------
```

### Load Upper Immediate

```text
-------------------------------------------------------------------------
|        |                                            |                 |
| itype  |                  imm40                     |      dest       |
|        |                                            |                 |
-------------------------------------------------------------------------
```

### Load Direct Immediate

```text
-------------------------------------------------------------------------
|        |        |                                   |                 |
| itype  |   0    |              imm32                |      dest       |
|        |        |                                   |                 |
-------------------------------------------------------------------------
```

### Load Null

```text
-------------------------------------------------------------------------
|        |                                            |                 |
| itype  |                     0                      |      dest       |
|        |                                            |                 |
-------------------------------------------------------------------------
```

## Numeric Instructions

---

* iadd
  * 2-input arithmetic operation
  * dest = src1 + src2
* ladd
  * 2-input arithmetic operation
  * dest = src1 + src2
* fadd
  * 2-input arithmetic operation
  * dest = src1 + src2
* dadd
  * 2-input arithmetic operation
  * dest = src1 + src2
* isub
  * 2-input arithmetic operation
  * dest = src1 - src2
* lsub
  * 2-input arithmetic operation
  * dest = src1 - src2
* fsub
  * 2-input arithmetic operation
  * dest = src1 - src2
* dsub
  * 2-input arithmetic operation
  * dest = src1 - src2
* imul
  * 2-input arithmetic operation
  * dest = src1 * src2
* lmul
  * 2-input arithmetic operation
  * dest = src1 * src2
* fmul
  * 2-input arithmetic operation
  * dest = src1 * src2
* dmul
  * 2-input arithmetic operation
  * dest = src1 * src2
* idiv
  * 2-input arithmetic operation
  * dest = src1 / src2
* ldiv
  * 2-input arithmetic operation
  * dest = src1 / src2
* fdiv
  * 2-input arithmetic operation
  * dest = src1 / src2
* ddiv
  * 2-input arithmetic operation
  * dest = src1 / src2
* imod
  * 2-input arithmetic operation
  * dest = src1 % src2
* lmod
  * 2-input arithmetic operation
  * dest = src1 % src2
* fmod
  * 2-input arithmetic operation
  * dest = src1 % src2
* dmod
  * 2-input arithmetic operation
  * dest = src1 % src2

## Bit Instructions

---

* iand
  * 2-input arithmetic operation
  * dest = src1 & src2
* land
  * 2-input arithmetic operation
  * dest = src1 & src2
* ior
  * 2-input arithmetic operation
  * dest = src1 | src2
* lor
  * 2-input arithmetic operation
  * dest = src1 | src2
* ixor
  * 2-input arithmetic operation
  * dest = src1 ^ src2
* lxor
  * 2-input arithmetic operation
  * dest = src1 ^ src2
* isll
  * 2-input arithmetic operation
  * dest = src1 << src2
* lsll
  * 2-input arithmetic operation
  * dest = src1 << src2
* isrl
  * 2-input arithmetic operation
  * dest = src1 >>> src2
* lsrl
  * 2-input arithmetic operation
  * dest = src1 >>> src2
* isra
  * 2-input arithmetic operation
  * dest = src1 >> src2
* lsra
  * 2-input arithmetic operation
  * dest = src1 >> src2

## Conversion Instructions

---

* itol
  * 1-input cast operation
  * long dest = int src
* itof
  * 1-input cast operation
  * float dest = int src
* itod
  * 1-input cast operation
  * double dest = int src
* ltoi
  * 1-input cast operation
  * int dest = long src
* ltof
  * 1-input cast operation
  * float dest = long src
* ltod
  * 1-input cast operation
  * double dest = long src
* ftoi
  * 1-input cast operation
  * int dest = float src
* ftol
  * 1-input cast operation
  * long dest = float src
* ftod
  * 1-input cast operation
  * double dest = float src
* dtoi
  * 1-input cast operation
  * int dest = double src
* dtol
  * 1-input cast operation
  * long dest = double src
* dtof
  * 1-input cast operation
  * float dest = double src
* iasf
  * 1-input cast operation
  * float dest = bit_cast&lt;float&gt;(int src)
* lasd
  * 1-input cast operation
  * double dest = bit_cast&lt;double&gt;(long src)
* fasi
  * 1-input cast operation
  * int dest = bit_cast&lt;int&gt;(float src)
* dasl
  * 1-input cast operation
  * long dest = bit_cast&lt;long&gt;(double src)

## Branch Instructions

---

* iblt
  * 2-input branch operation
  * if (int src1 &lt; int src2) goto dest
* ible
  * 2-input branch operation
  * if (int src1 &le; int src2) goto dest
* ibeq
  * 2-input branch operation
  * if (int src1 == int src2) goto dest
* lblt
  * 2-input branch operation
  * if (long src1 &lt; long src2) goto dest
* lble
  * 2-input branch operation
  * if (long src1 &le; long src2) goto dest
* lbeq
  * 2-input branch operation
  * if (long src1 == long src2) goto dest
* fblt
  * 2-input branch operation
  * if (float src1 &lt; float src2) goto dest
* fble
  * 2-input branch operation
  * if (float src1 &le; float src2) goto dest
* fbeq
  * 2-input branch operation
  * if (float src1 == float src2) goto dest
* dblt
  * 2-input branch operation
  * if (double src1 &lt; double src2) goto dest
* dble
  * 2-input branch operation
  * if (double src1 &le; double src2) goto dest
* dbeq
  * 2-input branch operation
  * if (double src1 == double src2) goto dest
* rbeq
  * 2-input branch operation
  * if (ref src1 == ref src2) goto dest

## Numeric Immediate Instructions

---

* iaddi
  * 2-input arithmetic immediate operation
  * dest = src1 + imm24
* laddi
  * 2-input arithmetic immediate operation
  * dest = src1 + imm24
* faddi
  * 2-input arithmetic immediate operation
  * dest = src1 + imm24
* daddi
  * 2-input arithmetic immediate operation
  * dest = src1 * imm24
* imuli
  * 2-input arithmetic immediate operation
  * dest = src1 * imm24
* lmuli
  * 2-input arithmetic immediate operation
  * dest = src1 * imm24
* fmuli
  * 2-input arithmetic immediate operation
  * dest = src1 * imm24
* dmuli
  * 2-input arithmetic immediate operation
  * dest = src1 * imm24
* idivi
  * 2-input arithmetic immediate operation
  * dest = src1 / imm24
* ldivi
  * 2-input arithmetic immediate operation
  * dest = src1 / imm24
* fdivi
  * 2-input arithmetic immediate operation
  * dest = src1 / imm24
* ddivi
  * 2-input arithmetic immediate operation
  * dest = src1 / imm24
* imodi
  * 2-input arithmetic immediate operation
  * dest = src1 % imm24
* lmodi
  * 2-input arithmetic immediate operation
  * dest = src1 % imm24
* fmodi
  * 2-input arithmetic immediate operation
  * dest = src1 % imm24
* dmodi
  * 2-input arithmetic immediate operation
  * dest = src1 % imm24
* irsubi
  * 2-input arithmetic immediate operation
  * dest = imm24 - src1
* lrsubi
  * 2-input arithmetic immediate operation
  * dest = imm24 - src1
* frsubi
  * 2-input arithmetic immediate operation
  * dest = imm24 - src1
* drsubi
  * 2-input arithmetic immediate operation
  * dest = imm24 - src1
* irdivi
  * 2-input arithmetic immediate operation
  * dest = imm24 / src1
* lrdivi
  * 2-input arithmetic immediate operation
  * dest = imm24 / src1
* frdivi
  * 2-input arithmetic immediate operation
  * dest = imm24 / src1
* drdivi
  * 2-input arithmetic immediate operation
  * dest = imm24 / src1

## Bit Immediate Instructions

---

* iandi
  * 2-input arithmetic immediate operation
  * dest = src1 & imm24
* landi
  * 2-input arithmetic immediate operation
  * dest = src1 & imm24
* iori
  * 2-input arithmetic immediate operation
  * dest = src1 | imm24
* lori
  * 2-input arithmetic immediate operation
  * dest = src1 | imm24
* ixori
  * 2-input arithmetic immediate operation
  * dest = src1 ^ imm24
* lxori
  * 2-input arithmetic immediate operation
  * dest = src1 ^ imm24
* islli
  * 2-input arithmetic immediate operation
  * dest = src1 << imm24
* lslli
  * 2-input arithmetic immediate operation
  * dest = src1 << imm24
* isrli
  * 2-input arithmetic immediate operation
  * dest = src1 >>> imm24
* lsrli
  * 2-input arithmetic immediate operation
  * dest = src1 >>> imm24
* israi
  * 2-input arithmetic immediate operation
  * dest = src1 >> imm24
* lsrai
  * 2-input arithmetic immediate operation
  * dest = src1 >> imm24

## Branch Immediate Instructions

---

* iblti
  * if (int src1 &lt; imm24) goto dest
* iblei
  * if (int src1 &le; imm24) goto dest
* ibeqi
  * if (int src1 == imm24) goto dest
* lblti
  * if (long src1 &lt; imm24) goto dest
* lblei
  * if (long src1 &le; imm24) goto dest
* lbeqi
  * if (long src1 == imm24) goto dest
* fblti
  * if (float src1 &lt; imm24) goto dest
* fblei
  * if (float src1 &le; imm24) goto dest
* fbeqi
  * if (float src1 == imm24) goto dest
* dblti
  * if (double src1 &lt; imm24) goto dest
* dblei
  * if (double src1 &le; imm24) goto dest
* dbeqi
  * if (double src1 == imm24) goto dest
* bnull
  * if (ref src1 == null) goto dest
* bu
  * goto dest

## Load Immediate Instructions

---

* lui
  * Load Upper Immediate instruction
  * Zeroes lower 24 bits
* ldi
  * Load Direct Immediate instruction
  * Zeroes upper bits
* lnul
  * Load Null instruction
  * Null identity is unspecified
