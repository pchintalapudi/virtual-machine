#include "instructions.h"

#include <climits>

using namespace oops::instructions;

instruction::itype instruction::type_of() const {
    return static_cast<instruction::itype>(this->_type);
}

oops::stack_idx_t instruction::src1() const {
    return this->_src1;
}
oops::stack_idx_t instruction::src2() const {
    return this->_src2;
}
oops::stack_idx_t instruction::dest() const {
    return this->_dest;
}
oops::instr_idx_t instruction::target() const {
    return this->_dest;
}

namespace {
    constexpr std::int32_t sign_extend_imm24(std::int32_t imm24) {
        return imm24 << (sizeof(imm24) * CHAR_BIT - 24) >> (sizeof(imm24) * CHAR_BIT - 24);
    }
}

std::int32_t instruction::imm24() const {
    return sign_extend_imm24(static_cast<std::uint32_t>(this->_src2) << sizeof(this->_padding) * CHAR_BIT | this->_padding);
}
std::int32_t instruction::imm32() const {
    return static_cast<std::uint32_t>(this->_src1) << sizeof(this->_src2) * CHAR_BIT | static_cast<std::uint32_t>(this->_src2);
}
std::int64_t instruction::imm40() const {
    return static_cast<std::uint64_t>(this->_src1) << (sizeof(this->_src2) + sizeof(this->_padding)) * CHAR_BIT | static_cast<std::uint32_t>(this->_src2) << sizeof(this->_padding) * CHAR_BIT |  this->_padding;
}