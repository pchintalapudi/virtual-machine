#ifndef OOPS_CLASSLOADER_CLASSLOADER_H
#define OOPS_CLASSLOADER_CLASSLOADER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../classes/class.h"
#include "../classes/object.h"
#include "../memory/bump_allocator.h"
#include "instanceof_table.h"

namespace oops {
namespace gc {
class class_iterator;
}
namespace classloading {
class classloader {
 private:
  std::unordered_map<std::string, classes::clazz>
      cached_classes;
  std::unordered_set<std::string> loading_classes;
  instanceof_table instanceof_table;
  memory::bump_allocator metaspace;

 public:
  bool initialize(std::uintptr_t metaspace_size);

  std::optional<classes::clazz> load_class(classloading::raw_string str);
  std::optional<classes::clazz> load_class_unwrapped(const char* str, std::int32_t length);

  bool is_superclass(classes::clazz maybe_super, classes::clazz maybe_sub);

  std::optional<classes::clazz> impl_load_class(const char *cstr,
                                                std::int32_t length);

  gc::class_iterator begin();
  gc::class_iterator end();

  void destroy();
};
}  // namespace classloading
}  // namespace oops

#endif