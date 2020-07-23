#include "class_manager.h"
#include "../objects/objects.h"

#include <istream>

using namespace oops::interfaze;

std::optional<std::uint32_t> class_manager::lookup_interface_method(objects::method imethod, objects::base_object src)
{
    typedef std::pair<char *, std::uint32_t> inlined_lookup;
    typedef std::unordered_map<char *, std::uint32_t> mega_lookup;
    auto cls = src.get_clazz();
    if (auto it = this->interface_method_cache.find(imethod.unwrap()); it != this->interface_method_cache.end())
    {
        auto variant = it->second;
        if (std::holds_alternative<inlined_lookup>(variant))
        {
            auto pair = std::get<inlined_lookup>(variant);
            if (cls.unwrap() == pair.first)
            {
                return pair.second;
            }
            else
            {
                auto method_idx = cls.lookup_interface_method(imethod.name());
                if (method_idx)
                    this->interface_method_cache[imethod.unwrap()] = mega_lookup{{pair, {cls.unwrap(), *method_idx}}};
                return method_idx;
            }
        }
        else
        {
            auto lookup = std::get<mega_lookup>(variant);
            if (auto it = lookup.find(cls.unwrap()); it != lookup.end())
            {
                return it->second;
            }
            else
            {
                auto method_idx = cls.lookup_interface_method(imethod.name());
                if (method_idx)
                    lookup[cls.unwrap()] = *method_idx;
                return method_idx;
            }
        }
    }
    else
    {
        auto method_idx = cls.lookup_interface_method(imethod.name());
        if (method_idx)
            this->interface_method_cache[imethod.unwrap()] = inlined_lookup{cls.unwrap(), *method_idx};
        return method_idx;
    }
}

bool class_manager:: instanceof (objects::clazz src, objects::clazz test) const
{
    if (auto it = this->loaded_classes.find(src.unwrap()); it != this->loaded_classes.end())
    {
        auto impl_it = this->implemented.begin() + it->second.second;
        return std::binary_search(impl_it + 1, impl_it + *impl_it + 1, utils::pun_reinterpret<std::uintptr_t>(test.unwrap()));
    }
    return false;
}

namespace
{
    class class_file
    {
    private:
        char *memory_mapped_file;

        char *class_references_start() const;
        char *method_references_start() const;
        char *static_variables_start() const;
        char *instance_variables_start() const;
        char *bytecodes_start() const;
        char *string_pool_start() const;

    public:

        std::uint32_t static_method_count() const;
        std::uint32_t virtual_method_count() const;
        std::uint64_t loaded_classes_count() const;
        std::uint64_t loaded_methods_count() const;
        std::uint32_t static_variables_count() const;
        std::uint32_t instance_variables_count() const;
        std::uint64_t static_variables_size() const;
        std::uint64_t instance_variables_size() const;
        std::uint64_t bytecode_size() const;
        std::uint64_t string_pool_size() const;

        class class_references {
            private:
            char* start;
            public:
            std::uint64_t operator[](std::uint32_t class_offset) {
                return oops::utils::pun_read<std::uint64_t>(start + static_cast<std::uint64_t>(class_offset) * sizeof(std::uint64_t) + sizeof(std::uint32_t) * 2);
            }
        };

        class_references get_class_references() const;
    };
} // namespace

oops::objects::clazz class_manager::load_class(utils::ostring name)
{
    auto loaded = this->loaded_classes.find(name);
    if (loaded != this->loaded_classes.end())
    {
        return objects::clazz(loaded->second.first);
    }
}