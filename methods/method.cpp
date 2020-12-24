#include "method.h"

using namespace oops::methods;
namespace {
namespace header {
#define dumb_type(name, tp) \
  struct name {             \
    tp value;               \
  };
dumb_type(total_size, std::uint32_t);
dumb_type(double_offset, std::uint32_t);
dumb_type(long_offset, std::uint32_t);
dumb_type(float_offset, std::uint32_t);
dumb_type(int_offset, std::uint32_t);
dumb_type(short_offset, std::uint32_t);
dumb_type(byte_offset, std::uint32_t);
dumb_type(bytecode_offset, std::uint32_t);
dumb_type(additional_metadata, std::uint32_t);
dumb_type(bytecode_size, std::uint16_t);
dumb_type(argument_count, std::uint8_t);
}  // namespace header

template <typename of, typename T, typename... Args>
constexpr unsigned offset_of() {
  if constexpr (std::is_same_v<T, of>) {
    return 0;
  } else {
    return sizeof(T) + offset_of<of, Args...>();
  }
}
template <typename of, typename T, typename... Args>
constexpr unsigned index_of() {
  if constexpr (std::is_same_v<T, of>) {
    return 0;
  } else {
    return index_of<of, Args...>() + 1;
  }
}

template <typename of, typename... Args>
constexpr unsigned offset_of_v = offset_of<of, Args...>();

template <typename of, typename... Args>
constexpr unsigned index_of_v = index_of<of, Args...>();

#define hto(t) header::t
#define header_types                                                        \
  hto(total_size), hto(double_offset), hto(long_offset), hto(float_offset), \
      hto(int_offset), hto(short_offset), hto(byte_offset),                 \
      hto(bytecode_offset), hto(additional_metadata), hto(bytecode_size),   \
      hto(argument_count)
template <typename htype>
using header_type_of = decltype(htype::value);

template <typename htype, typename... Args>
using end_type_of =
    std::tuple_element_t<index_of_v<htype, Args...> + 1, std::tuple<Args...>>;

constexpr unsigned total_header_size =
    offset_of_v<header::argument_count, header_types> +
    sizeof(header_type_of<header::argument_count>);
}  // namespace

method::method(void *ptr) { this->location.initialize(ptr); }

void *method::get_raw() const { return this->location.get_raw(); }

oops::instructions::instruction method::read_instruction(
    instr_idx_t offset) const {
  auto bytecode_start =
      this->location.read<header_type_of<header::bytecode_offset>>(
          offset_of_v<header::bytecode_offset, header_types>);
  return instructions::instruction(this->location.read<std::uint64_t>(
      bytecode_start +
      static_cast<std::uint32_t>(offset) * sizeof(std::uint64_t)));
}

std::uint16_t method::stack_frame_size() const {
  return this->location.read<header_type_of<header::total_size>>(
      offset_of_v<header::total_size, header_types>);
}

std::array<std::uint16_t, 7> method::get_bounds() const {
  return {pointer_offset(), double_offset(), long_offset(), float_offset(),
          int_offset(),     short_offset(),  byte_offset()};
}

#define offset_func(type)                                              \
  std::uint16_t method::type##_offset() const {                        \
    return this->location.read<header_type_of<header::type##_offset>>( \
        offset_of_v<header::type##_offset, header_types>);             \
  }
offset_func(byte);
offset_func(short);
offset_func(int);
offset_func(float);
offset_func(long);
offset_func(double);

std::uint8_t method::arg_count() const {
  return this->location.read<header_type_of<header::argument_count>>(
      offset_of_v<header::argument_count, header_types>);
}
arg_types method::get_arg_types() const {
  return arg_types(
      this->arg_count(),
      static_cast<char *>(this->location.get_raw()) + total_header_size);
}

args method::get_args_for_called_instruction(instr_idx_t call_instr_idx,
                                             std::uint8_t nargs) const {
  auto bytecode_start =
      this->location.read<header_type_of<header::bytecode_offset>>(
          offset_of_v<header::bytecode_offset, header_types>);
  auto args_offset =
      bytecode_start +
      static_cast<std::uint32_t>(call_instr_idx + 1) * sizeof(std::uint64_t);
  return args(nargs,
              static_cast<char *>(this->location.get_raw()) + args_offset);
}