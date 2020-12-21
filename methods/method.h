#ifndef OOPS_METHODS_METHOD_H
#define OOPS_METHODS_METHOD_H

#include "../classes/datatypes.h"
#include "../instructions/instructions.h"
#include "../memory/byteblock.h"

namespace oops {
namespace methods {

class args {
 private:
  memory::byteblock<false> arg_location;
  unsigned count;

 public:
  args(std::uint8_t length, void *args) {
    count = length;
    this->arg_location.initialize(args);
  }

  std::uint8_t length() const { return count; }

  stack_idx_t operator[](std::uint16_t idx) const {
    return arg_location.read<std::uint8_t>(idx);
  }
};

class arg_types {
 private:
  memory::byteblock<false> arg_location;
  unsigned count;

 public:
  arg_types(std::uint8_t length, void *args) {
    count = length;
    this->arg_location.initialize(args);
  }

  std::uint8_t length() const { return count; }

  classes::datatype operator[](std::uint16_t idx) const {
    std::uint8_t packed_type = arg_location.read<std::uint8_t>(idx / 2);
    packed_type >>= idx % 2 ? 4 : 0;
    return static_cast<classes::datatype>(packed_type & 0b111);
  }
};

class method {
 public:
  enum class mtype { STANDARD, NATIVE };

 private:
  memory::byteblock<false> location;
  std::uintptr_t instruction_offset(instr_idx_t offset) const;

 public:
  method(void *ptr);
  instructions::instruction read_instruction(instr_idx_t offset) const;

  std::uint16_t stack_frame_size() const;

  std::uint16_t pointer_count() const;
  std::uint16_t double_count() const;
  std::uint16_t long_count() const;
  std::uint16_t float_count() const;
  std::uint16_t int_count() const;
  std::uint16_t short_count() const;
  std::uint16_t byte_count() const;

  std::uint8_t arg_count() const;
  arg_types get_arg_types() const;

  args get_args_for_called_instruction(instr_idx_t call_instr_idx,
                                       std::uint8_t nargs) const;
};
}  // namespace methods
}  // namespace oops

#endif