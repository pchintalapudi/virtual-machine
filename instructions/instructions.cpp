#include "instructions.h"

#include <climits>

using namespace oops::instructions;

instruction::itype instruction::type_of() const {
  return static_cast<instruction::itype>(this->base & 0xff);
}

oops::stack_idx_t instruction::src1() const { return this->base >> 32 & 0xffff; }
oops::stack_idx_t instruction::src2() const { return this->base >> 16 & 0xffff; }
oops::stack_idx_t instruction::dest() const { return this->base >> 48 & 0xffff; }
oops::instr_idx_t instruction::target() const { return this->base >> 48 & 0xffff; }

namespace {
constexpr std::int32_t sign_extend_imm24(std::int32_t imm24) {
  return imm24 << (sizeof(imm24) * CHAR_BIT - 24) >>
         (sizeof(imm24) * CHAR_BIT - 24);
}
}  // namespace

std::int32_t instruction::imm24() const {
  return sign_extend_imm24(this->base >> 8 & 0xffffff);
}
std::uint32_t instruction::idx24() const {
  return this->base >> 8 & 0xffffff;
}
std::int32_t instruction::imm32() const {
  return this->base >> 16 & 0xffffffff;
}
std::int64_t instruction::imm40() const {
  return this->base >> 8  & 0xffffffffff;
}