#ifndef OOPS_CLASSLOADER_CLASSLOADER_H
#define OOPS_CLASSLOADER_CLASSLOADER_H

#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../classes/class.h"
#include "../classes/object.h"
#include "../memory/bump_allocator.h"
#include "../native/args.h"
#include "instanceof_table.h"

namespace oops {
namespace gc {
class class_iterator;
}
struct bootstrapped_classes {
  std::array<classes::clazz, 7> array_classes;
  classes::clazz string_class;
  classes::clazz object_class;
};
namespace classloading {
class classloader {
 private:
  std::unordered_map<std::string, classes::clazz> cached_classes;
  std::unordered_set<std::string> loading_classes;
  instanceof_table instanceof_table;
  memory::bump_allocator metaspace;
  classloader_options options;
  bootstrapped_classes core_classes;

 public:
  bool initialize(const classloader_options &args);

  std::optional<classes::clazz> load_class(classloading::raw_string str);
  std::optional<classes::clazz> load_class_unwrapped(const char *str,
                                                     std::int32_t length);

  bool is_superclass(classes::clazz maybe_super, classes::clazz maybe_sub);

  std::optional<classes::clazz> impl_load_class(const char *cstr,
                                                std::int32_t length);

  classes::clazz string_class() { return this->core_classes.string_class; }
  classes::clazz array_class(classes::datatype array_type) {
    return this->core_classes.array_classes[static_cast<unsigned>(array_type)];
  }

  gc::class_iterator begin();
  gc::class_iterator end();

  void destroy();
};
}  // namespace classloading
}  // namespace oops

#endif