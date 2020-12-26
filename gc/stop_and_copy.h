#ifndef OOPS_GC_STOP_AND_COPY_H
#define OOPS_GC_STOP_AND_COPY_H

#include "../classes/object.h"
#include "../memory/ostack.h"

#include <atomic>
#include <unordered_map>

namespace oops {
namespace gc {
void *forward_object(classes::base_object root, void *destination,
                     void **new_location);
void *track_class_pointers(classes::clazz cls, void *destination);
void *track_stack_pointers(memory::stack *stack, void *destination);
void *track_native_pointers(
    std::unordered_map<void *, std::pair<oops_object_t*, std::atomic_uint64_t>>
        &native_pointers,
    decltype(native_pointers) scratch_map, void *destination);
void *cleanup_migrated_objects(void *begin, void *end);
}  // namespace gc
}  // namespace oops

#endif