#include "instanceof_table.h"

#include <algorithm>

using namespace oops::classloading;

bool instanceof_table::is_superclass(classes::clazz super, classes::clazz sub) {
  auto &index_and_length = this->class_indeces[sub.get_self_index()];
  auto begin = this->superclasses.begin() + index_and_length.first;
  auto end = begin + index_and_length.second;
  auto present = std::lower_bound(begin, end, super.get_raw());
  return present != end && *present == super.get_raw();
}

std::uint32_t instanceof_table::insert_index(std::uint32_t) {
  auto idx = this->class_indeces.size();
  this->class_indeces.emplace_back(this->superclasses.size(), 0);
  return idx;
}
void instanceof_table::insert_superclass(void *supcls) {
  this->class_indeces.back().second++;
  this->superclasses.push_back(supcls);
}
bool instanceof_table::commit_superclasses() {
  std::sort(this->superclasses.begin() + this->class_indeces.back().first,
            this->superclasses.begin() + (this->class_indeces.back().first +
                                          this->class_indeces.back().second));
  return true;
}
void instanceof_table::pop_last_class() {
  this->superclasses.resize(this->class_indeces.back().first);
  this->class_indeces.pop_back();
}