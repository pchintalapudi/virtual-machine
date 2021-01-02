#ifndef OOPS_MEMORY_OHEAP_H
#define OOPS_MEMORY_OHEAP_H

#include <atomic>
#include <unordered_map>
#include <unordered_set>

#include "../classes/class.h"
#include "../classes/datatypes.h"
#include "../classes/object.h"
#include "../classloader/classloader.h"
#include "../gc/semispace_allocator.h"
#include "../native/native_types.h"

namespace oops {
namespace memory {
class stack;
class heap {
 private:
  std::unordered_map<void *, std::pair<oops_object_t *, std::atomic_uint64_t>>
      native_references;
  decltype(heap::native_references) scratch;
  std::unordered_set<stack *> vm_stacks;
  classloading::classloader bootstrap_classloader;

  gc::semispace allocator;

  std::optional<void *> allocate_memory(std::uintptr_t amount);

 public:
  bool initialize(std::uintptr_t max_size);
  std::optional<classes::object> allocate_object(classes::clazz);
  std::optional<classes::array> allocate_array(classes::datatype dt,
                                               std::int32_t length);
  std::optional<classes::base_object> reify_constant_string(classloading::raw_string str);

  void register_stack(stack *stack);
  void unregister_stack(stack *stack);
  oops_object_t *allocate_native_reference(classes::base_object obj);
  void deallocate_native_reference(oops_object_t *obj);
  classloading::classloader *get_classloader() {
    return &this->bootstrap_classloader;
  }

  void destroy();
};
}  // namespace memory
}  // namespace oops

#endif