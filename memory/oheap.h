#ifndef OOPS_MEMORY_OHEAP_H
#define OOPS_MEMORY_OHEAP_H

#include <array>
#include <atomic>
#include <unordered_map>
#include <unordered_set>

#include "../classes/class.h"
#include "../classes/datatypes.h"
#include "../classes/object.h"
#include "../classloader/classloader.h"
#include "../gc/semispace_allocator.h"
#include "../native/args.h"
#include "../native/native_types.h"

namespace oops {
class virtual_machine;
namespace core {
class executor;
}
namespace memory {
class stack;
class heap {
 private:
  std::unordered_map<void *, std::pair<oops_object_t *, std::atomic_uint64_t>>
      native_references;
  decltype(heap::native_references) scratch;
  std::unordered_set<core::executor *> vm_executors;

  gc::semispace allocator;
  classloading::classloader bootstrap_classloader;

  std::optional<void *> allocate_memory(std::uintptr_t amount);

  void register_executor(core::executor *executor);
  void unregister_executor(core::executor *executor);

  friend class oops::virtual_machine;

 public:
  bool initialize(const heap_options &heap_args);
  std::optional<classes::object> allocate_object(classes::clazz);
  std::optional<classes::array> allocate_array(classes::datatype dt,
                                               std::int32_t length);
  std::optional<classes::base_object> reify_constant_string(
      classloading::raw_string str);
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