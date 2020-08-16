#include "instruction.h"

#include <climits>
#include "../utils/utils.h"
using namespace oops::bytecode;

instruction::instruction(char *real)
{
    std::uint64_t instr = utils::pun_read<std::uint64_t>(real);
    this->udest = instr & 0xffffull;
    this->usrc1 = (instr >>= sizeof(this->udest) * CHAR_BIT) & 0xffffull;
    this->usrc2 = (instr >>= sizeof(this->usrc1) * CHAR_BIT) & 0xffffull;
    this->uflags = (instr >>= sizeof(this->usrc2) * CHAR_BIT) & 0xffull;
    this->opcode = (instr >>= sizeof(this->uflags) * CHAR_BIT) & 0xffull;
}

instruction::type instruction::get_type()
{
    return static_cast<type>(this->opcode);
}

std::uint16_t instruction::src1()
{
    return this->usrc1;
}
std::uint16_t instruction::src2()
{
    return this->usrc2;
}
std::int16_t instruction::imm2()
{
    return this->usrc2;
}
std::uint16_t instruction::dest()
{
    return this->udest;
}
std::uint8_t instruction::flags()
{
    return this->uflags;
}
std::int32_t instruction::imm24()
{
    return static_cast<std::int32_t>(this->uflags) << sizeof(std::uint16_t) * CHAR_BIT | this->usrc2;
}
std::int32_t instruction::imm32()
{
    return static_cast<std::int32_t>(this->usrc2) << sizeof(std::uint16_t) * CHAR_BIT | this->usrc1;
}