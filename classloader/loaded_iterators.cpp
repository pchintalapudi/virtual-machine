#include "loaded_iterators.h"

#include <climits>

#include "../classes/class_header.h"

using namespace oops::classloading;

loaded_class_reference_iterator::reference
loaded_class_reference_iterator::operator*() {
  return reference(this);
}

loaded_import_reference_iterator::reference
loaded_import_reference_iterator::operator*() {
  return reference(this);
}

loaded_field_reference_iterator::reference
loaded_field_reference_iterator::operator*() {
  return reference(this);
}

bdr_reference<class_reference> &bdr_reference<class_reference>::operator=(
    const class_reference &ref) {
  this->it->backing->cls.write(
      this->it->index,
      this->it->backing->translate_string_index(ref.string_idx));
  return *this;
}

bdr_reference<import_reference> &bdr_reference<import_reference>::operator=(
    const import_reference &ref) {
  auto backing = this->it->backing;
  backing->cls.write(this->it->index,
                     backing->translate_string_index(ref.string_idx));
  backing->cls.write(this->it->index + sizeof(std::uint32_t),
                     ref.class_reference);
  return *this;
}

bdr_reference<field>::operator field() const {
  field out;
  auto backing = this->it->backing;
  auto string_pool_offset =
      backing->cls.read<decltype(classes::class_header::string_pool_offset)>(
          offsetof(classes::class_header, string_pool_offset));
  auto offset = this->it->index;
  out.string_idx = backing->cls.read<std::uint32_t>(offset);
  out.data_idx =
      backing->cls.read<std::uint32_t>(offset + sizeof(std::uint32_t));
  out.data_type = out.data_idx >> CHAR_BIT * 3;
  out.data_idx &= 0x00'FF'FF'FF;
  out.name = *classes::clazz(backing->cls.get_raw())
                  .load_constant_string(out.string_idx);
  out.string_idx -= string_pool_offset;
  return out;
}

bdr_reference<field> &bdr_reference<field>::operator=(const field &f) {
  auto backing = this->it->backing;
  std::uint32_t string_idx = backing->translate_string_index(f.string_idx);
  std::uint32_t data_idx = f.data_type;
  data_idx <<= CHAR_BIT * 3;
  data_idx |= f.data_idx;
  backing->cls.write<std::uint32_t>(this->it->index, string_idx);
  backing->cls.write<std::uint32_t>(this->it->index + sizeof(std::uint32_t), data_idx);
  return *this;
}