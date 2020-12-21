#ifndef OOPS_CLASSLOADER_CLASSLOADER_H
#define OOPS_CLASSLOADER_CLASSLOADER_H

#include <unordered_map>

#include "../classes/class.h"
#include "../classes/object.h"

namespace oops {
namespace classloading {
class classloader {
 private:
  std::unordered_map<char *, classes::clazz> cached_classes;

 public:
  std::optional<classes::clazz> load_class(classes::string str);
};
}  // namespace classloading
}  // namespace oops

#endif