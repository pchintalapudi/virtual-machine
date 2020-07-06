#include "heap.h"

using namespace oops::memory;

std::optional<oops::objects::object> heap::allocate_object(oops::objects::clazz cls) {
    if (cls.object_malloc_required_size() < this->max_young_object_size) {
        return this->allocate_object_young(cls);
    } else {
        return this->allocate_object_old(cls);
    }
}