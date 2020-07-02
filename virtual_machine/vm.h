#ifndef VIRTUAL_MACHINE_VM
#define VIRTUAL_MACHINE_VM

#include "../objects/objects.h"

namespace oops {
    namespace vm {
        class virtual_machine {
            private:

            public:

            int execute(objects::method method);
        };
    }
}

#endif /* VIRTUAL_MACHINE_VM */
