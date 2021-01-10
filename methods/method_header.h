#ifndef OOPS_METHODS_METHOD_HEADER_H
#define OOPS_METHODS_METHOD_HEADER_H

#include <cstdint>

namespace oops {
namespace methods {

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
  std::uint32_t name;
  std::uint32_t total_method_size;
  void *context_class;
  std::uint8_t argument_count;
  std::uint8_t arg_types[variable_length];
};
#undef variable_length
}  // namespace methods
}  // namespace oops

#endif
