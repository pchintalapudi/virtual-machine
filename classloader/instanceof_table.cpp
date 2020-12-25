#include "instanceof_table.h"

using namespace oops::classloading;

bool instanceof_table::is_superclass(classes::clazz super, classes::clazz sub) {
  auto index_and_length = this->class_indeces.find(sub.get_raw());
  if (index_and_length == this->class_indeces.end()) {
    return false;
  }
  auto begin = this->classes.begin() + sub.get_self_index();
  auto end = begin + index_and_length->second;
  auto present = std::lower_bound(begin, end, super.get_raw());
  return present != end && *present == super.get_raw();
}
