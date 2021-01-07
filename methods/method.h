#ifndef OOPS_METHODS_METHOD_H
#define OOPS_METHODS_METHOD_H

#include <array>

#include "../classes/datatypes.h"
#include "../instructions/instructions.h"
#include "../memory/byteblock.h"

namespace oops {
namespace classes {
class clazz;
}
namespace methods {

class args {
 private:
  memory::byteblock<false> arg_location;
  unsigned count;

 public:
  args(std::uint8_t length, const void *args) {
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
  enum struct type { OBJECT, DOUBLE, LONG, FLOAT, INT };
  arg_types(std::uint8_t length, const void *args) {
    count = length;
    this->arg_location.initialize(args);
  }

  std::uint8_t length() const { return count; }

  type operator[](std::uint16_t idx) const {
    std::uint8_t packed_type = arg_location.read<std::uint8_t>(idx / 2);
    packed_type >>= idx % 2 ? 4 : 0;
    return static_cast<type>(packed_type & 0b111);
  }
};

class method {
 public:
  enum class mtype { STANDARD, NATIVE };

 private:
  memory::byteblock<false> location;

 public:
  method(void *ptr);
  const void *get_raw() const;
  instructions::instruction read_instruction(instr_idx_t offset) const;

  std::uint16_t stack_frame_size() const;

  std::array<std::uint16_t, 5> get_bounds() const;
  std::uint16_t pointer_offset() const { return 0; }
  std::uint16_t double_offset() const;
  std::uint16_t long_offset() const;
  std::uint16_t float_offset() const;
  std::uint16_t int_offset() const;

  std::uint8_t arg_count() const;
  arg_types get_arg_types() const;

  args get_args_for_called_instruction(instr_idx_t call_instr_idx,
                                       std::uint8_t nargs) const;

  classes::string get_name() const;
  classes::clazz get_context_class() const;
};
}  // namespace methods
}  // namespace oops

#endif