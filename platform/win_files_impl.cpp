#include "files.h"

using namespace oops::platform;

#ifdef OOPS_COMPILE_FOR_WINDOWS
#include <fileapi.h>
#include <handleapi.h>
#include <windows.h>

#include <string>

#include "files.h"

std::optional<oops::platform::mmap_file> mmap_file::create(const char *filename,
                                                           int length) {
  mmap_file_impl impl;
  std::string file(filename, length);
  impl.file_handle = CreateFile(file.c_str(), GENERIC_READ, 0, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (impl.file_handle != INVALID_HANDLE_VALUE) {
    LARGE_INTEGER temp;
    GetFileSizeEx(impl.file_handle, &temp);
    impl.file_size = temp.QuadPart;
    if (impl.file_size != 0) {
      impl.file_mapping_handle = CreateFileMapping(
          impl.file_handle, NULL, PAGE_READONLY, 0, 0, file.c_str());
      if (impl.file_mapping_handle != NULL) {
        impl.file_view =
            MapViewOfFile(impl.file_mapping_handle, FILE_MAP_READ, 0, 0, 0);
        if (impl.file_view != NULL) {
          return mmap_file(impl);
        }
        CloseHandle(impl.file_mapping_handle);
      }
    }
    CloseHandle(impl.file_handle);
  }
  return {};
}
const void* mmap_file::operator*() const { return this->impl.file_view; }
std::uintptr_t mmap_file::file_size() const { return this->impl.file_size; }
mmap_file::~mmap_file() {
  if (this->impl.file_size) {
    UnmapViewOfFile(this->impl.file_view);
    CloseHandle(this->impl.file_mapping_handle);
    CloseHandle(this->impl.file_handle);
    this->impl.file_size = 0;
  }
}

mmap_file::mmap_file(mmap_file &&file) {
  std::swap(this->impl, file.impl);
  file.impl.file_size = 0;
}
mmap_file &mmap_file::operator=(mmap_file &&file) {
  std::swap(this->impl, file.impl);
  return *this;
}
#endif