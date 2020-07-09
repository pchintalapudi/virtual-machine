#include "heap.h"

using namespace oops::memory;

std::optional<oops::objects::object> heap::allocate_object(oops::objects::clazz cls) {
    if (cls.object_malloc_required_size() < this->max_young_object_size) {
        return this->young_generation.allocate_object(cls);
    } else {
        return this->old_generation.allocate_object(cls);
    }
}

std::optional<oops::objects::array> heap::allocate_array(oops::objects::clazz acls, std::uint64_t memory_size) {
    if (memory_size < this->max_young_object_size) {
        return this->young_generation.allocate_array(acls, memory_size);
    } else {
        return this->old_generation.allocate_array(acls, memory_size);
    }
}