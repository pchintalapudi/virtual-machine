#ifndef BYTECODE_INSTRUCTION
#define BYTECODE_INSTRUCTION

#include <cstdint>

namespace oops {
    namespace bytecode {
        class instruction {
            private:
            public:
            enum class type : unsigned char {
                NOP,
                ADD,
                SUB,
                MUL,
                DIV,
                DIVU,
                ADDI,
                SUBI,
                MULI,
                DIVI,
                DIVUI,
                LUI,
                LLI,
                CST,
                AND,
                OR,
                XOR,
                SLL,
                SRL,
                SRA,
                ANDI,
                ORI,
                XORI,
                SLLI,
                SRLI,
                SRAI,
                BGE,
                BLT,
                BLE,
                BGT,
                BEQ,
                BNEQ,
                BGEI,
                BLTI,
                BLEI,
                BGTI,
                BEQI,
                BNEQI,
                BCMP,
                BADR,
                BU,
                VLLD,
                VLSR,
                ALD,
                ASR,
                VNEW,
                ANEW,
                IOF,
                VINV,
                SINV,
                IINV,
                LVINV,
                LIINV,
                RET
            };
            type get_type();

            std::uint16_t src1();
            std::uint16_t src2();
            std::uint16_t dest();
        };
    }
}

#endif /* BYTECODE_INSTRUCTION */
