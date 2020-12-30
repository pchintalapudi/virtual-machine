#include "stop_and_copy.h"

#include <atomic>
#include <unordered_map>

#include "../native/native_types.h"
#include "class_iterators.h"
#include "stack_iterators.h"

using namespace oops::gc;

void *oops::gc::forward_object(classes::base_object root, void *destination,
                               void **new_location, void *low_bound,
                               void *high_bound) {
  // No need to trace out-of-bounds, we're done here
  if (root.get_raw() < low_bound || root.get_raw() >= high_bound) {
    *new_location = root.get_raw();
    return destination;
  }
  auto obj_pointer = root.get_raw();
  std::uintptr_t pointer;
  memcpy(&pointer, obj_pointer, sizeof(void *));
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
  *new_location = static_cast<char *>(destination) + sizeof(std::uint64_t);
  return static_cast<char *>(destination) + total_size;
}
void *oops::gc::track_class_pointers(classes::clazz cls, void *vdestination,
                                     void *low_bound, void *high_bound) {
  char *destination = static_cast<char *>(vdestination);
  for (auto str : cls.strings()) {
    void *forwarded;
    destination = static_cast<char *>(
        forward_object(str, destination, &forwarded, low_bound, high_bound));
    str = classes::base_object(forwarded);
  }
  for (auto ptr : cls.static_pointers()) {
    void *forwarded;
    destination = static_cast<char *>(
        forward_object(ptr, destination, &forwarded, low_bound, high_bound));
    ptr = classes::base_object(forwarded);
  }
  return destination;
}
void *oops::gc::track_stack_pointers(memory::stack *stack, void *vdestination,
                                     void *low_bound, void *high_bound) {
  char *destination = static_cast<char *>(vdestination);
  for (auto frame : *stack) {
    for (auto pointer : frame) {
      void *forwarding_pointer;
      destination = static_cast<char *>(forward_object(
          pointer, destination, &forwarding_pointer, low_bound, high_bound));
      pointer = classes::base_object(forwarding_pointer);
    }
  }
  return destination;
}
void *oops::gc::cleanup_migrated_objects(void *vbegin, void *vend,
                                         void *low_bound, void *high_bound) {
  char *begin = static_cast<char *>(vbegin), *end = static_cast<char *>(vend);
  begin += sizeof(std::uint64_t);
  while (begin < end) {
    auto obj = classes::base_object(begin);
    if (obj.is_array()) {
      auto array = obj.as_array();
      if (array.element_type() == classes::datatype::OBJECT) {
        for (std::int32_t i = 0; i < array.length(); i++) {
          void *forwarding_pointer;
          end = static_cast<char *>(
              forward_object(*array.get<classes::base_object>(i), end,
                             &forwarding_pointer, low_bound, high_bound));
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
        end = static_cast<char *>(
            forward_object(*object.read<classes::base_object>(i), end,
                           &forwarding_pointer, low_bound, high_bound));
        object.write<void *>(i, forwarding_pointer);
      }
      begin += cls.object_size() + sizeof(std::uint64_t);
    }
  }
  return end;
}
void *oops::gc::track_native_pointers(
    std::unordered_map<void *, std::pair<oops_object_t *, std::atomic_uint64_t>>
        &native_pointers,
    decltype(native_pointers) scratch_map, void *destination, void *low_bound,
    void *high_bound) {
  scratch_map.clear();
  scratch_map.reserve(native_pointers.size());
  for (auto &obj : native_pointers) {
    void *new_location;
    destination = forward_object(classes::base_object(obj.first), destination,
                                 &new_location, low_bound, high_bound);
    auto &ref = scratch_map[new_location];
    ref.first = obj.second.first;
    ref.second = obj.second.second.load();
    ref.first->object = new_location;
  }
  std::swap(scratch_map, native_pointers);
  return destination;
}