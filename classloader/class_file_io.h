#ifndef OOPS_CLASSLOADER_CLASS_FILE_IO_H
#define OOPS_CLASSLOADER_CLASS_FILE_IO_H

#include <array>
#include <cstdint>

#include "../classes/class.h"
#include "../memory/bump_allocator.h"
#include "../platform/files.h"

namespace oops {
namespace classloading {
struct raw_string {
  const char *const str;
  const std::int32_t length;
};

class class_reference_iterator;
class instance_field_reference_iterator;
class static_field_reference_iterator;
class method_iterator;
template<typename it>
class class_iterable;

class class_file_reader {
 private:
  std::optional<platform::mmap_file> backing;
  memory::byteblock<false> file;
  std::uint32_t class_reference_table_offset() const;
  std::uint32_t instance_field_table_offset() const;
  std::uint32_t static_field_table_offset() const;
  std::uint32_t method_table_offset() const;
  std::uint32_t string_pool_offset() const;

  friend class class_reference_iterator;
  friend class instance_field_reference_iterator;
  friend class static_field_reference_iterator;
  friend class method_iterator;

 public:
  // Handled
  bool initialize(const char *filename, std::int32_t length);
  std::uint16_t magic_number() const;
  std::uint16_t implemented_count() const;
  // Unhandled
  std::uint32_t total_class_file_size() const;
  class_iterable<class_reference_iterator> class_references();
  class_iterable<class_reference_iterator> instance_field_references();
  class_iterable<class_reference_iterator> static_field_references();
  class_iterable<class_reference_iterator> methods();
  // Handled
  void destroy();
};

class class_writer {
 public:
  // Unhandled
  bool initialize(memory::bump_allocator &allocator, std::uintptr_t size);
  // Handled
  void set_superclass(classes::clazz superclass);
  void set_static_variable_offsets(std::array<std::uint32_t, 7> offsets);
  void set_instance_variable_offsets(std::array<std::uint32_t, 7> offsets);
  // Unhandled
  void set_vmt_offset(std::uint32_t offset);
  void set_class_descriptor_table_offset(std::uint32_t offset);
  void set_field_descriptor_table_offset(std::uint32_t offset);
  void set_reflection_table_offset(std::uint32_t offset);
  void set_method_bytecodes_offset(std::uint32_t offset);
  // Handled
  void set_class_index(std::uint32_t index);
  void revoke();
  classes::clazz commit();
};
}  // namespace classloading
}  // namespace oops

#endif