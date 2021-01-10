#include "../classloader/classloader.h"

#include "../memory/oheap.h"
#include "../native/args.h"

using namespace oops::classloading;

void classloader::destroy() {
  this->cached_classes.clear();
  this->instanceof_table.destroy();
  this->metaspace.destroy();
}

bool classloader::initialize(
    const classloader_options &args) {
  this->options = args;
  if (this->metaspace.initialize(args.metaspace_size)) {
    // TODO load bootstrapped classes
    this->metaspace.destroy();
  }
  return {};
}