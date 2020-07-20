#ifndef INTERFACE_INTERFACE_CACHE
#define INTERFACE_INTERFACE_CACHE

#include <map>
#include <variant>
#include "instanceof.h"

namespace oops
{
    namespace interfaze
    {
        class class_manager
        {
        private:
            instanceof _instanceof;
            std::unordered_map<char *, std::variant<std::pair<char *, std::uint32_t>, std::unordered_map<char *, std::uint32_t>>> interface_method_cache;
            char *base, *head, *committed;
            std::uint64_t allocation_granularity;
            std::map<utils::ostring, char*> loaded_classes;

        public:
            bool instanceof (objects::clazz src, objects::clazz test) const
            {
                return this->_instanceof(src, test);
            }

            std::optional<std::uint32_t> lookup_interface_method(objects::method imethod, objects::base_object src);

            objects::clazz load_class(utils::ostring clazz);
        };
    } // namespace interfaze
} // namespace oops

#endif /* INTERFACE_INTERFACE_CACHE */
