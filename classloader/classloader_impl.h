#ifndef OOPS_CLASSLOADER_CLASSLOADER_IMPL_H
#define OOPS_CLASSLOADER_CLASSLOADER_IMPL_H

#include "../classes/class.h"
#include "../classes/object.h"
#include "../memory/bump_allocator.h"

namespace oops {
namespace classloading {
std::optional<classes::clazz> impl_load_class(memory::bump_allocator &metaspace, char *cstr, std::int32_t length);
}
}  // namespace oops

#endif