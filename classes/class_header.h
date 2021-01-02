#ifndef OOPS_CLASSES_CLASS_HEADER_H
#define OOPS_CLASSES_CLASS_HEADER_H

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace oops {
namespace classes {
namespace header {
#define dumb_type(name, tp) \
  struct name {             \
    tp value;               \
  };
dumb_type(superclass, void *);
dumb_type(static_total_size, std::uint32_t);
dumb_type(static_double_offset, std::uint32_t);
dumb_type(static_long_offset, std::uint32_t);
dumb_type(static_float_offset, std::uint32_t);
dumb_type(static_int_offset, std::uint32_t);
dumb_type(static_short_offset, std::uint32_t);
dumb_type(static_byte_offset, std::uint32_t);
dumb_type(instance_total_size, std::uint32_t);
dumb_type(instance_double_offset, std::uint32_t);
dumb_type(instance_long_offset, std::uint32_t);
dumb_type(instance_float_offset, std::uint32_t);
dumb_type(instance_int_offset, std::uint32_t);
dumb_type(instance_short_offset, std::uint32_t);
dumb_type(instance_byte_offset, std::uint32_t);
dumb_type(vmt_offset, std::uint32_t);
dumb_type(class_import_offset, std::uint32_t);
dumb_type(field_import_offset, std::uint32_t);
dumb_type(exports_offset, std::uint32_t);
dumb_type(bytecodes_offset, std::uint32_t);
dumb_type(string_pool_offset, std::uint32_t);
dumb_type(class_size, std::uint32_t);
dumb_type(class_index, std::uint32_t);
#undef dumb_type
};  // namespace header

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

#define chto(t) oops::classes::header::t
#define class_header_types                                                \
  chto(superclass), chto(static_total_size), chto(static_double_offset),  \
      chto(static_long_offset), chto(static_float_offset),                \
      chto(static_int_offset), chto(static_short_offset),                 \
      chto(static_byte_offset), chto(instance_total_size),                \
      chto(instance_double_offset), chto(instance_long_offset),           \
      chto(instance_float_offset), chto(instance_int_offset),             \
      chto(instance_short_offset), chto(instance_byte_offset),            \
      chto(vmt_offset), chto(class_import_offset),                    \
      chto(field_import_offset), chto(exports_offset),            \
      chto(bytecodes_offset), chto(string_pool_offset), chto(class_size), \
      chto(class_index)
template <typename htype>
using header_type_of = decltype(htype::value);

template <typename htype, typename... Args>
using end_type_of =
    std::tuple_element_t<index_of_v<htype, Args...> + 1, std::tuple<Args...>>;

constexpr unsigned total_header_size =
    offset_of_v<header::class_index, class_header_types> +
    sizeof(header_type_of<header::class_index>);
}  // namespace classes
#undef dumb_type
}  // namespace oops

#endif