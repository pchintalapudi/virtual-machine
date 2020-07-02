#ifndef OBJECTS_FIELD
#define OBJECTS_FIELD

#include <cstdint>
#include "../utils/utils.h"

namespace oops {
    namespace objects {

        class field {
            private:
            char* real;
            public:
            enum class type {
                CHAR, SHORT, INT, LONG, FLOAT, DOUBLE, OBJECT
            };

            type get_type() const;

            std::uint16_t object_offset() const;

            utils::ostring name() const;
        };
    }
}

#endif /* OBJECTS_FIELD */
