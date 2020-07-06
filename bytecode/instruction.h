#ifndef BYTECODE_INSTRUCTION
#define BYTECODE_INSTRUCTION

#include <cstdint>

namespace oops
{
    namespace bytecode
    {
        class instruction
        {
        private:
        public:
            enum class type : unsigned char
            {
                //Done
                NOP,
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
                IDIVU,
                LDIVU,
                IADDI,
                LADDI,
                FADDI,
                DADDI,
                ISUBI,
                LSUBI,
                FSUBI,
                DSUBI,
                IMULI,
                LMULI,
                FMULI,
                DMULI,
                IDIVI,
                LDIVI,
                FDIVI,
                DDIVI,
                IDIVUI,
                LDIVUI,
                INEG,
                LNEG,
                FNEG,
                DNEG,
                LUI,
                LLI,
                LNL,
                ICSTL,
                ICSTF,
                ICSTD,
                LCSTI,
                LCSTF,
                LCSTD,
                FCSTI,
                FCSTL,
                FCSTD,
                DCSTI,
                DCSTL,
                DCSTF,
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
                IBGE,
                LBGE,
                FBGE,
                DBGE,
                IBLT,
                LBLT,
                FBLT,
                DBLT,
                IBLE,
                LBLE,
                FBLE,
                DBLE,
                IBGT,
                LBGT,
                FBGT,
                DBGT,
                IBEQ,
                LBEQ,
                FBEQ,
                DBEQ,
                VBEQ,
                IBNEQ,
                LBNEQ,
                FBNEQ,
                DBNEQ,
                VBNEQ,
                IBGEI,
                LBGEI,
                FBGEI,
                DBGEI,
                IBLTI,
                LBLTI,
                FBLTI,
                DBLTI,
                IBLEI,
                LBLEI,
                FBLEI,
                DBLEI,
                IBGTI,
                LBGTI,
                FBGTI,
                DBGTI,
                IBEQI,
                LBEQI,
                FBEQI,
                DBEQI,
                VBEQI,
                IBNEQI,
                LBNEQI,
                FBNEQI,
                DBNEQI,
                VBNEQI,
                IBCMP,
                LBCMP,
                FBCMP,
                DBCMP,
                BADR,
                BU,
                CVLLD,
                SVLLD,
                IVLLD,
                LVLLD,
                FVLLD,
                DVLLD,
                VVLLD,
                CVLSR,
                SVLSR,
                IVLSR,
                LVLSR,
                FVLSR,
                DVLSR,
                VVLSR,
                CALD,
                SALD,
                IALD,
                LALD,
                FALD,
                DALD,
                VALD,
                CASR,
                SASR,
                IASR,
                LASR,
                FASR,
                DASR,
                VASR,
                CSTLD,
                SSTLD,
                ISTLD,
                LSTLD,
                FSTLD,
                DSTLD,
                VSTLD,
                CSTSR,
                SSTSR,
                ISTSR,
                LSTSR,
                FSTSR,
                DSTSR,
                VSTSR,
                VNEW,
                CANEW,
                SANEW,
                IANEW,
                LANEW,
                FANEW,
                DANEW,
                VANEW,
                IOF,
                LIOF,
                //TODO
                VINV,
                SINV,
                IINV,
                IRET,
                LRET,
                FRET,
                DRET,
                EXC
            };
            
            instruction(char* istr);

            type get_type();

            std::uint16_t src1();
            std::uint16_t src2();
            std::int16_t imm2();
            std::uint16_t dest();
            std::uint8_t flags();
            std::int32_t imm24();
            std::int32_t imm32();
        };
    } // namespace bytecode
} // namespace oops

#endif /* BYTECODE_INSTRUCTION */
