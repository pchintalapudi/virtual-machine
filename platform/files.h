#ifndef OOPS_PLATFORM_FILES_H
#define OOPS_PLATFORM_FILES_H

#include "platform_selector.h"

#ifdef OOPS_COMPILE_FOR_WINDOWS
#include "win_mmap_file_impl.h"
#endif

#include <cstdint>
#include <optional>

namespace oops {
namespace platform {
class mmap_file {
 private:
  mmap_file_impl impl;
  mmap_file(mmap_file_impl impl) : impl(impl) {}

 public:
  mmap_file(const mmap_file &) = delete;
  mmap_file &operator=(const mmap_file &) = delete;
  mmap_file(mmap_file &&file);
  mmap_file &operator=(mmap_file &&file);
  static std::optional<mmap_file> create(const char *filename, int length);
  operator void *();
  std::uintptr_t file_size();
  ~mmap_file();
};
}  // namespace platform
}  // namespace oops

#endif