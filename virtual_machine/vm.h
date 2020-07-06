#ifndef VIRTUAL_MACHINE_VM
#define VIRTUAL_MACHINE_VM

#include "../memory/memory.h"

namespace oops {
    namespace virtual_machine {
        class virtual_machine {
            private:

            char* ip;
            memory::frame frame;
            memory::stack stack;
            memory::heap heap;

            int exec_loop();

            objects::clazz current_class();

            bool instanceof(objects::clazz base, objects::clazz subclass);

            std::optional<objects::method> lookup_interface_method(objects::method imethod, objects::base_object src);

            public:

            int execute(objects::method method);
        };
    }
}

#endif /* VIRTUAL_MACHINE_VM */
