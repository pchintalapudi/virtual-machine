#include "object.h"

#include <climits>

#include "class.h"

using namespace oops::classes;

base_object::base_object(void *obj) { this->data.initialize(obj); }

array::array(void *arr) { this->data.initialize(arr); }

object::object(void *obj) { this->data.initialize(obj); }

base_object array::to_base_object() const {
  return base_object(this->data.get_raw());
}

base_object object::to_base_object() const {
  return base_object(this->data.get_raw());
}

array base_object::as_array() const { return array(this->data.get_raw()); }

object base_object::as_object() const { return object(this->data.get_raw()); }

std::int32_t array::length() const {
  return static_cast<std::int32_t>(this->data.read<std::int64_t>(0) >>
                                   sizeof(std::int32_t) * CHAR_BIT);
}

datatype array::element_type() const {
  return static_cast<datatype>(this->data.read<std::int64_t>(0) >> 1 & 0b111);
}

bool base_object::is_null() const { return this->data.get_raw() == nullptr; }

bool base_object::is_array() const {
  return this->data.read<std::int64_t>(0) & 1;
}

void *base_object::get_raw() const { return this->data.get_raw(); }

void array::initialize(std::int32_t length, datatype dt) {
  std::int64_t header = 0;
  header |= length;
  header <<= (sizeof(length) * CHAR_BIT - 1);
  header |= static_cast<std::uint64_t>(dt);
  header <<= 1;
  header |= 1;
  this->data.write(0, header);
}