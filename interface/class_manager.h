#ifndef INTERFACE_INTERFACE_CACHE
#define INTERFACE_INTERFACE_CACHE

#include <cstdint>
#include <unordered_map>
#include <variant>
#include <vector>
#include "../utils/utils.h"
#include "../objects/objects.h"

namespace oops
{
    namespace interfaze
    {
        class class_manager
        {
        private:
            std::unordered_map<char *, std::variant<std::pair<char *, std::uint32_t>, std::unordered_map<char *, std::uint32_t>>> interface_method_cache;
            char *base, *head, *cap;
            std::vector<std::uintptr_t> implemented;
            std::unordered_map<utils::ostring, std::pair<char*, std::size_t>> loaded_classes;

        public:

            struct args {
                std::uint64_t min_size, max_size;
            };

            bool init(args& init_args);
            
            bool instanceof (objects::clazz src, objects::clazz test) const;

            std::optional<std::uint32_t> lookup_interface_method(objects::method imethod, objects::base_object src);

            objects::clazz load_class(utils::ostring clazz);

            void deinit();
        };
    } // namespace interfaze
} // namespace oops

#endif /* INTERFACE_INTERFACE_CACHE */
