#ifndef OOPS_NATIVE_ARGS_H
#define OOPS_NATIVE_ARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
struct classloader_options {
  uintptr_t metaspace_size;
  const char *root_package_directory;
};
struct heap_options {
  uintptr_t min_heap_size;
  uintptr_t max_heap_size;
  double min_heap_saturation;
  double max_heap_saturation;
  struct classloader_options classloader_args;
};
struct executor_options {
  uintptr_t default_stack_size;
};

struct vm_options {
  struct heap_options heap_args;
  struct executor_options executor_args;
};

#ifdef __cplusplus
}
#endif

#endif