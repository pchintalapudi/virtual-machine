#ifndef OOPS_CLASSLOADER_INSTANCEOF_TABLE_H
#define OOPS_CLASSLOADER_INSTANCEOF_TABLE_H

#include <vector>

#include "../classes/class.h"

namespace oops {
namespace classloading {
class instanceof_table {
 private:
  std::vector<std::pair<std::uint32_t, std::uint32_t>> class_indeces;
  std::vector<void *> superclasses;

 public:
  std::uint32_t insert_index(std::uint32_t length);
  void insert_superclass(void* supcls);
  bool commit_superclasses();
  void pop_last_class();
  bool is_superclass(classes::clazz super, classes::clazz sub);
  void destroy();
};
}  // namespace classloading
}  // namespace oops

#endif