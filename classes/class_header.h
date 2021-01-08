#ifndef OOPS_CLASSES_CLASS_HEADER_H
#define OOPS_CLASSES_CLASS_HEADER_H

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace oops {
namespace classes {
struct class_header {
  void *superclass;

  std::uint32_t static_total_size;
  std::uint32_t static_double_offset;
  std::uint32_t static_long_offset;
  std::uint32_t static_float_offset;
  std::uint32_t static_int_offset;
  std::uint32_t static_short_offset;
  std::uint32_t static_byte_offset;
  std::uint32_t instance_total_size;
  std::uint32_t instance_double_offset;
  std::uint32_t instance_long_offset;
  std::uint32_t instance_float_offset;
  std::uint32_t instance_int_offset;
  std::uint32_t instance_short_offset;
  std::uint32_t instance_byte_offset;

  std::uint32_t vmt_offset;
  std::uint32_t class_import_offset;
  std::uint32_t field_import_offset;
  std::uint32_t exports_offset;
  std::uint32_t bytecodes_offset;
  std::uint32_t string_pool_offset;

  std::uint32_t class_size;
  std::uint32_t class_index;
};
}  // namespace classes
}  // namespace oops

#endif