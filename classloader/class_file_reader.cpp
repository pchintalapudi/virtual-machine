#include "class_file_io.h"

using namespace oops::classloading;

  bool class_file_reader::initialize(const char *filename, std::int32_t length) {
      this->file = platform::mmap_file::create(filename, length);
      return this->file.has_value();
  }
  void class_file_reader::destroy() {
      this->file.reset();
  }
