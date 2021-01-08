#include "method.h"

#include "../classes/class.h"

using namespace oops::methods;
namespace {
#define variable_length 1
struct method_header {
  std::uint16_t total_size;
  std::uint16_t double_offset;
  std::uint16_t long_offset;
  std::uint16_t float_offset;
  std::uint16_t int_offset;
  std::uint16_t additional_metadata;
  std::uint16_t bytecode_offset;
  std::uint16_t bytecode_size;
  void *name;
  void *context_class;
  std::uint8_t argument_count;
  std::uint8_t arg_types[variable_length];
};
#undef variable_length
}  // namespace

method::method(const void *ptr) { this->location.initialize(ptr); }

const void *method::get_raw() const { return this->location.get_raw(); }

#define read_header(field)                             \
  this->location.read<decltype(method_header::field)>( \
      offsetof(method_header, field))

oops::instructions::instruction method::read_instruction(
    instr_idx_t offset) const {
  auto bytecode_start = read_header(bytecode_offset);
  return instructions::instruction(this->location.read<std::uint64_t>(
      bytecode_start +
      static_cast<std::uint32_t>(offset) * sizeof(std::uint64_t)));
}

std::uint16_t method::stack_frame_size() const {
  return read_header(total_size);
}

std::array<std::uint16_t, 5> method::get_bounds() const {
  return {pointer_offset(), double_offset(), long_offset(), float_offset(),
          int_offset()};
}

#define offset_func(type)                       \
  std::uint16_t method::type##_offset() const { \
    return read_header(type##_offset);          \
  }
offset_func(int);
offset_func(float);
offset_func(long);
offset_func(double);

std::uint8_t method::arg_count() const { return read_header(argument_count); }
arg_types method::get_arg_types() const {
  return arg_types(this->arg_count(),
                   static_cast<const char *>(this->location.get_raw()) +
                       offsetof(method_header, arg_types));
}

args method::get_args_for_called_instruction(instr_idx_t call_instr_idx,
                                             std::uint8_t nargs) const {
  auto bytecode_start = read_header(bytecode_offset);
  auto args_offset =
      bytecode_start +
      static_cast<std::uint32_t>(call_instr_idx + 1) * sizeof(std::uint64_t);
  return args(
      nargs, static_cast<const char *>(this->location.get_raw()) + args_offset);
}

oops::classes::string method::get_name() {
  return classes::string(read_header(name));
}

oops::classes::clazz method::get_context_class() {
  return classes::clazz(read_header(context_class));
}