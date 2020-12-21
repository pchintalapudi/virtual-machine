#ifndef OOPS_CORE_EXECUTOR_H
#define OOPS_CORE_EXECUTOR_H

#include "../classloader/classloader.h"
#include "../classloader/instanceof_table.h"
#include "../globals/types.h"
#include "../memory/oheap.h"
#include "../memory/ostack.h"
#include "../methods/method.h"
#include "../native/native_types.h"

namespace oops {
namespace core {

class executor {
 private:
  memory::stack vm_stack;
  memory::heap *vm_heap;
  classloading::classloader bootstrap_classloader;
  classloading::instanceof_table instanceof_table;

 public:
  bool initialize(const executor_options &options);

  oops_wrapper_t invoke(classes::clazz context, methods::method method, const oops_wrapper_t *args,
                        int nargs);

  void destroy();
};
}  // namespace core
}  // namespace oops

#endif /* CORE_EXECUTOR */
