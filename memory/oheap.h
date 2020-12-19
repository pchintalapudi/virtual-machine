#ifndef OOPS_MEMORY_OHEAP_H
#define OOPS_MEMORY_OHEAP_H

#include "../classes/class.h"

namespace oops {
    namespace memory {
        class heap {
            private:
            public:

            std::optional<void*> allocate_memory(classes::clazz);
        };
    }
}

#endif