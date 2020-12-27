#ifndef OOPS_CLASSLOADER_CLASS_FILE_IO_H
#define OOPS_CLASSLOADER_CLASS_FILE_IO_H

#include <cstdint>

#include "../memory/bump_allocator.h"
#include "../platform/files.h"
#include "loader_iterators.h"

namespace oops {
namespace classloading {
class raw_string;

class class_file_reader {
 private:
  std::optional<platform::mmap_file> file;
  std::uint32_t class_reference_table_offset() const;
  std::uint32_t instance_field_table_offset() const;
  std::uint32_t static_field_table_offset() const;
  std::uint32_t method_table_offset() const;
  std::uint32_t string_pool_offset() const;

 public:
  bool initialize(const char *filename, std::int32_t length);
  std::uint16_t magic_number() const;
  std::uint32_t total_class_size() const;
  class_iterable<class_reference_iterator> class_references();
  class_iterable<class_reference_iterator> instance_field_references();
  class_iterable<class_reference_iterator> static_field_references();
  class_iterable<class_reference_iterator> methods();
  void destroy();
};

class class_writer {
 public:
  bool initialize(memory::bump_allocator &allocator, std::uintptr_t size);
  void revoke();
};
}  // namespace classloading
}  // namespace oops

#endif