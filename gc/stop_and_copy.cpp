#include "stop_and_copy.h"
#include "stack_iterators.h"

using namespace oops::gc;

void *oops::gc::forward_object(classes::base_object root, void *destination,
                               void **new_location) {
  // No need to trace nulls, we're done here
  if (root.is_null()) {
    *new_location = nullptr;
    return destination;
  }
  auto obj_pointer = root.get_raw();
  std::uint64_t pointer;
  memcpy(&pointer, &destination, sizeof(destination));
  // Object was already copied
  if (pointer & 0b10) {
    pointer &= ~0b10ull;
    memcpy(new_location, &pointer, sizeof(void *));
    return destination;
  }
  std::uint64_t total_size;
  if (root.is_array()) {
    auto array = root.as_array();
    auto dt = array.element_type();
    total_size =
        classes::datatype_size(dt) * array.length() + sizeof(std::uint64_t);
    memcpy(destination,
           static_cast<char *>(obj_pointer) - sizeof(std::uint64_t),
           total_size);
  } else {
    auto obj = root.as_object();
    total_size = obj.get_class().object_size() + sizeof(std::uint64_t);
    memcpy(destination,
           static_cast<char *>(obj_pointer) - sizeof(std::uint64_t),
           total_size);
  }
  pointer |= 0b10;
  memcpy(obj_pointer, &pointer, sizeof(pointer));
  *new_location = destination;
  return static_cast<char *>(destination) + total_size;
}
void *oops::gc::track_class_pointers(classes::clazz cls, void *vdestination);
void *oops::gc::track_stack_pointers(memory::stack *stack, void *vdestination) {
    char* destination = static_cast<char*>(vdestination);
    for (auto frame : *stack) {
        for (auto pointer : frame) {
            void* forwarding_pointer;
            destination = static_cast<char*>(forward_object(pointer, destination, &forwarding_pointer));
            pointer = classes::base_object(forwarding_pointer);
        }
    }
    return destination;
}
void *oops::gc::cleanup_migrated_objects(void *vbegin, void *vend) {
  char *begin = static_cast<char *>(vbegin), *end = static_cast<char *>(vend);
  begin += sizeof(std::uint64_t);
  while (begin < end) {
    auto obj = classes::base_object(begin);
    if (obj.is_array()) {
      auto array = obj.as_array();
      if (array.element_type() == classes::datatype::OBJECT) {
        for (std::int32_t i = 0; i < array.length(); i++) {
          void *forwarding_pointer;
          end = static_cast<char *>(forward_object(
              *array.get<classes::base_object>(i), end, &forwarding_pointer));
          array.set(i, classes::base_object(forwarding_pointer));
        }
      }  // no other array type can reference other objects
      begin += array.length() * classes::datatype_size(array.element_type()) +
               sizeof(std::uint64_t);
    } else {
      auto object = obj.as_object();
      auto cls = object.get_class();
      auto end_pointer_offset = cls.instance_double_offset();
      for (unsigned i = 0; i < end_pointer_offset; i += sizeof(void *)) {
        void *forwarding_pointer;
        end = static_cast<char *>(forward_object(
            *object.read<classes::base_object>(i), end, &forwarding_pointer));
        object.write<void *>(i, forwarding_pointer);
      }
      begin += cls.object_size() + sizeof(std::uint64_t);
    }
  }
  return end;
}