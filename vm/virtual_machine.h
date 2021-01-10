#ifndef OOPS_VM_VIRTUAL_MACHINE_H
#define OOPS_VM_VIRTUAL_MACHINE_H

#include "../core/executor.h"
#include "../memory/oheap.h"
#include "../native/args.h"

namespace oops {
class virtual_machine {
 private:
 vm_options options;
 memory::heap vm_heap;

 public:
  std::optional<core::executor *> initialize(const vm_options &args);
  void dispose(core::executor *executor);
  void destroy();
};
}  // namespace oops

#endif