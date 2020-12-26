#ifndef OOPS_CLASSLOADER_CLASSLOADER_H
#define OOPS_CLASSLOADER_CLASSLOADER_H

#include <unordered_map>

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
  std::unordered_map<char *, classes::clazz> cached_classes;
  instanceof_table instanceof_table;
  memory::bump_allocator metaspace;

 public:
  std::optional<classes::clazz> load_class(classes::string str);

  bool is_superclass(classes::clazz maybe_super, classes::clazz maybe_sub);

  gc::class_iterator begin();
  gc::class_iterator end();
};
}  // namespace classloading
}  // namespace oops

#endif