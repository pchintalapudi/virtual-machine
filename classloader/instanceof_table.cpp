#include "instanceof_table.h"

using namespace oops::classloading;

bool instanceof_table::is_superclass(classes::clazz super, classes::clazz sub) {
  auto &index_and_length = this->class_indeces[sub.get_self_index()];
  auto begin = this->superclasses.begin() + index_and_length.first;
  auto end = begin + index_and_length.second;
  auto present = std::lower_bound(begin, end, super.get_raw());
  return present != end && *present == super.get_raw();
}