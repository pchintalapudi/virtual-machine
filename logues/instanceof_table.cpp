#include "../classloader/instanceof_table.h"

using namespace oops::classloading;

void instanceof_table::destroy() {
  this->class_indeces.clear();
  this->superclasses.clear();
}