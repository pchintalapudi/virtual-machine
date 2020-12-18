#include "object.h"
#include "class.h"

using namespace oops::classes;

base_object::base_object(void* obj) {
    this->data.initialize(obj);
}

array::array(void* arr) {
    this->data.initialize(arr);
}

object::object(void* obj) {
    this->data.initialize(obj);
}

base_object array::to_base_object() const {
    return base_object(this->data.get_raw());
}

base_object object::to_base_object() const {
    return base_object(this->data.get_raw());
}

array base_object::as_array() const {
    return array(this->data.get_raw());
}

object base_object::as_object() const {
    return object(this->data.get_raw());
}

std::int32_t array::length() const {
    return this->data.read<std::int32_t>(0);
}

datatype array::element_type() const {
    return static_cast<datatype>(this->data.read<std::int32_t>(sizeof(std::int32_t)));
}

bool base_object::is_null() const {
    return this->data.get_raw() == nullptr;
}

void* base_object::get_raw() const {
    return this->data.get_raw();
}