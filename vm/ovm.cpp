#include "ovm.h"
#include "../punning/puns.h"
#include "primitive_ops.h"

using namespace oops::vm;

namespace
{
    enum class OPCODE
    {
        LLI = 0x60,
        CAST,
        NEG,
        DIVUI,
        DIVI,
        MULI,
        ADDI,
        DIVU,
        DIV,
        MUL,
        SUB,
        ADD,
        SRAI = 0x50,
        SRLI,
        SLLI,
        XORI,
        ORI,
        ANDI,
        SRA,
        SRL,
        SLL,
        XOR,
        OR,
        AND,
        JUMP = 0x40,
        BA,
        BNEQI,
        BEQI,
        BGTI,
        BLEI,
        BLTI,
        BGEI,
        BNEQ,
        BEQ,
        BGT,
        BLE,
        BLT,
        BGE,
        POPHDR = 0x10,
        PUSHHDR,
        THROW,
        NOP = 0x00,
        RET,
        ARGS,
        VINV,
        SINV,
        IINV,
        NINV,
        DINV,
        STLD,
        STSR,
        VLLD,
        VLSR
    };
    typedef oops::objects::field::field_type type;
} // namespace

int virtual_machine::execute()
{
    PUN(std::uint64_t, instr, next_instruction.back());
    std::uint16_t dest = instr >> 48u & 0xffffull,
                  src1 = instr >> (sizeof(std::uint16_t) << 4u) & 0xffffull,
                  src2 = instr >> (sizeof(std::uint16_t) << 3u) & 0xffffull;
    std::uint8_t typeinfo = instr >> (sizeof(uint8_t) << 3u) & 0xffull,
                 opcode = 0xffull;
    ::type t1 = static_cast<::type>(typeinfo & 0b111),
           t2 = static_cast<::type>(typeinfo >> 3 & 0b111);

    switch (static_cast<OPCODE>(opcode))
    {
#define op case OPCODE::
        //Arithmetic
        op ADD : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::add(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SUB : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::sub(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op MUL : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::mul(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIV : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_stack_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::div(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIVU : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::divu(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op ADDI : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::add(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op MULI : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::mul(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIVI : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::div(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op DIVUI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::divu(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op NEG : if (!this->read_stack_primitive([this, dest](auto arg1) { return this->writeback(primitives::neg(arg1), dest); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op CAST : if (!this->read_stack_primitive([this, src2, t2, dest](auto arg1) { return this->read_imm_primitive([this, arg1, dest](auto arg2) { return this->writeback(primitives::cast<decltype(arg1), decltype(arg2)>(arg1), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op LLI : if (!this->read_imm_primitive_32([this, dest](auto imm) { return this->writeback(imm, dest); }, static_cast<std::uint32_t>(src1) << 16 | src2, t1)) return this->invalid_bytecode(instr);
        break;
        //Bit operations
        op AND : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_and(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op OR : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_or(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op XOR : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_xor(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SLL : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sll(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRL : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_srl(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRA : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_stack_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sra(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op ANDI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_and(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op ORI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_or(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op XORI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_xor(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SLLI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sll(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRLI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_srl(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        op SRAI : if (!this->read_stack_integer([this, src2, t2, dest](auto arg1) { return this->read_imm_integer([this, arg1, dest](auto arg2) { return this->writeback(primitives::bit_sra(arg1, arg2), dest); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        break;
        //Branches
        op BGE : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::ge(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLT : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::lt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLE : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::le(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BGT : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_stack_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::gt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BGEI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::ge(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLTI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::lt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BLEI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::le(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BGTI : if (!this->read_stack_primitive([this, src2, t2, dest, typeinfo](auto arg1) { return this->read_imm_primitive([this, arg1, dest, typeinfo](auto arg2) { return this->branch(primitives::gt(arg1, arg2), dest, typeinfo >> 6 & 1); }, src2, t2); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op BEQ : if (!this->decode_eq([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 == arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BNEQ : if (!this->decode_eq([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 != arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BEQI : if (!this->decode_eq_imm([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 == arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BNEQI : if (!this->decode_eq_imm([this, dest, typeinfo](auto arg1, auto arg2) { return this->branch(arg1 != arg2, dest, typeinfo >> 6 & 1); }, src1, t1, src2, t2)) return this->invalid_bytecode(instr);
        return 0;
        op BA : if (!this->read_stack_integer([this, typeinfo](auto offset) { return this->jump(offset, typeinfo >> 6 & 1); }, src1, t1)) return this->invalid_bytecode(instr);
        return 0;
        op JUMP : this->jump(dest, typeinfo >> 6 & 1);
        return 0;
        //Objects
        //Exceptions
        //Methods
        op NOP : break;
    }
    this->next_instruction.back() += sizeof(std::uint64_t);
    return 0;
#undef INTERPRET
}