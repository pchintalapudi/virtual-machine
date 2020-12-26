#include "../classloader/classloader.h"

using namespace oops::classloading;

void classloader::destroy() {
  this->cached_classes.clear();
  this->instanceof_table.destroy();
  this->metaspace.destroy();
}

bool classloader::initialize(std::uintptr_t max_size) {
  return this->metaspace.initialize(max_size);
}