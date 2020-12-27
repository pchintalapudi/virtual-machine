#include "classloader_impl.h"

#include "class_file_io.h"

using namespace oops::classloading;

std::optional<oops::classes::clazz> oops::classloading::impl_load_class(
    memory::bump_allocator &metaspace, char *cstr, std::int32_t length) {
  class_file_reader reader;
  if (!reader.initialize(cstr, length)) {
    return {};
  }
  class_writer writer;
  //TODO calculate sizes
  if (!writer.initialize(metaspace, ~0ull)) {
      goto failure_after_reader_init;
  }
  {
  }
failure:
  writer.revoke();
failure_after_reader_init:
  reader.destroy();
  return {};
}