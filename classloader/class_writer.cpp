#include "../classes/class_header.h"
#include "../classes/datatypes.h"
#include "class_file_io.h"

using namespace oops::classloading;

#define write_header(htype, value)                                      \
  this->cls.write(                                                      \
      classes::offset_of_v<classes::header::htype, class_header_types>, \
      static_cast<classes::header_type_of<classes::header::htype>>(value))

bool class_writer::initialize(memory::bump_allocator &allocator,
                              std::uintptr_t size) {
  auto allocated = allocator.allocate(size);
  this->allocated = size;
  this->cls.initialize(*allocated);
  return !!allocated;
}
void class_writer::set_superclass(classes::clazz superclass) {
  write_header(superclass, superclass.get_raw());
}
void class_writer::set_static_variable_offsets(
    std::array<std::uint32_t, 7> offsets) {
  write_header(static_byte_offset,
               offsets[static_cast<unsigned>(classes::datatype::BYTE)]);
  write_header(static_short_offset,
               offsets[static_cast<unsigned>(classes::datatype::SHORT)]);
  write_header(static_int_offset,
               offsets[static_cast<unsigned>(classes::datatype::INT)]);
  write_header(static_float_offset,
               offsets[static_cast<unsigned>(classes::datatype::FLOAT)]);
  write_header(static_long_offset,
               offsets[static_cast<unsigned>(classes::datatype::LONG)]);
  write_header(static_double_offset,
               offsets[static_cast<unsigned>(classes::datatype::DOUBLE)]);
  write_header(static_total_size,
               offsets[static_cast<unsigned>(classes::datatype::OBJECT)]);
}
void class_writer::set_instance_variable_offsets(
    std::array<std::uint32_t, 7> offsets) {
  write_header(instance_byte_offset,
               offsets[static_cast<unsigned>(classes::datatype::BYTE)]);
  write_header(instance_short_offset,
               offsets[static_cast<unsigned>(classes::datatype::SHORT)]);
  write_header(instance_int_offset,
               offsets[static_cast<unsigned>(classes::datatype::INT)]);
  write_header(instance_float_offset,
               offsets[static_cast<unsigned>(classes::datatype::FLOAT)]);
  write_header(instance_long_offset,
               offsets[static_cast<unsigned>(classes::datatype::LONG)]);
  write_header(instance_double_offset,
               offsets[static_cast<unsigned>(classes::datatype::DOUBLE)]);
  write_header(instance_total_size,
               offsets[static_cast<unsigned>(classes::datatype::OBJECT)]);
}
void class_writer::set_vmt_offset(std::uint32_t offset) {
  write_header(vmt_offset, offset);
}
void class_writer::set_class_import_table_offset(std::uint32_t offset) {
  write_header(class_import_offset, offset);
}
void class_writer::set_field_import_table_offset(std::uint32_t offset) {
  write_header(field_import_offset, offset);
}
void class_writer::set_export_table_offset(std::uint32_t offset) {
  write_header(exports_offset, offset);
}
void class_writer::set_method_bytecodes_offset(std::uint32_t offset) {
  write_header(bytecodes_offset, offset);
}
void class_writer::set_class_index(std::uint32_t index) {
  write_header(class_index, index);
}
void class_writer::revoke(memory::bump_allocator &allocator) {
  allocator.deallocate(this->allocated);
}
oops::classes::clazz class_writer::commit() {
  return classes::clazz(this->cls.get_raw());
}