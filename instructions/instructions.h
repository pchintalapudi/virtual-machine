#ifndef OOPS_INSTRUCTIONS_INSTRUCTIONS_H
#define OOPS_INSTRUCTIONS_INSTRUCTIONS_H

#include <type_traits>

#include "../globals/types.h"

namespace oops {
namespace instructions {

class instruction {
 public:
  enum struct itype {
    IADD,
    LADD,
    FADD,
    DADD,
    ISUB,
    LSUB,
    FSUB,
    DSUB,
    IMUL,
    LMUL,
    FMUL,
    DMUL,
    IDIV,
    LDIV,
    FDIV,
    DDIV,
    IMOD,
    LMOD,
    FMOD,
    DMOD,

    IAND,
    LAND,
    IOR,
    LOR,
    IXOR,
    LXOR,
    ISLL,
    LSLL,
    ISRL,
    LSRL,
    ISRA,
    LSRA,

    ITOL,
    ITOF,
    ITOD,
    LTOI,
    LTOF,
    LTOD,
    FTOI,
    FTOL,
    FTOD,
    DTOI,
    DTOL,
    DTOF,
    IASF,
    LASD,
    FASI,
    DASL,

    IBLT,
    IBLE,
    IBEQ,
    LBLT,
    LBLE,
    LBEQ,
    FBLT,
    FBLE,
    FBEQ,
    DBLT,
    DBLE,
    DBEQ,
    RBEQ,

    IADDI,
    LADDI,
    FADDI,
    DADDI,
    IMULI,
    LMULI,
    FMULI,
    DMULI,
    IDIVI,
    LDIVI,
    FDIVI,
    DDIVI,
    IMODI,
    LMODI,
    FMODI,
    DMODI,
    IRSUBI,
    LRSUBI,
    FRSUBI,
    DRSUBI,
    IRDIVI,
    LRDIVI,
    FRDIVI,
    DRDIVI,

    IANDI,
    LANDI,
    IORI,
    LORI,
    IXORI,
    LXORI,
    ISLLI,
    LSLLI,
    ISRLI,
    LSRLI,
    ISRAI,
    LSRAI,

    IBLTI,
    IBLEI,
    IBEQI,
    LBLTI,
    LBLEI,
    LBEQI,
    FBLTI,
    FBLEI,
    FBEQI,
    DBLTI,
    DBLEI,
    DBEQI,
    BNULL,
    BU,

    LUI,
    LDI,
    LNUL,
    LCS,

    CALD,
    SALD,
    IALD,
    LALD,
    FALD,
    DALD,
    RALD,

    COLD,
    SOLD,
    IOLD,
    LOLD,
    FOLD,
    DOLD,
    ROLD,

    CCLD,
    SCLD,
    ICLD,
    LCLD,
    FCLD,
    DCLD,
    RCLD,

    CASR,
    SASR,
    IASR,
    LASR,
    FASR,
    DASR,
    RASR,

    COSR,
    SOSR,
    IOSR,
    LOSR,
    FOSR,
    DOSR,
    ROSR,

    CCSR,
    SCSR,
    ICSR,
    LCSR,
    FCSR,
    DCSR,
    RCSR,

    ONEW,
    CANEW,
    SANEW,
    IANEW,
    LANEW,
    FANEW,
    DANEW,
    RANEW,

    IOF,

    SCALL,
    VCALL,
    DCALL,

    CRET,
    SRET,
    IRET,
    LRET,
    FRET,
    DRET,
    RRET,
    NRET,

    EXC
  };

 private:
 std::uint64_t base;

 public:

 instruction(std::uint64_t instr) : base(instr) {}
  itype type_of() const;

  stack_idx_t src1() const;
  stack_idx_t src2() const;
  stack_idx_t dest() const;
  instr_idx_t target() const;

  std::int32_t imm24() const;
  std::uint32_t idx24() const;
  std::int32_t imm32() const;
  std::int64_t imm40() const;
};
static_assert(std::is_standard_layout_v<instruction>);
}  // namespace instructions
}  // namespace oops

#endif