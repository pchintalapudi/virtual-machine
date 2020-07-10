#include "interface_cache.h"
#include "../objects/objects.h"

using namespace oops::interfaze;

std::optional<std::uint32_t> class_manager::lookup_interface_method(objects::method imethod, objects::base_object src)
{
    typedef std::pair<char*, std::uint32_t> inlined_lookup;
    typedef std::unordered_map<char*, std::uint32_t> mega_lookup;
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
                if (method_idx) this->interface_method_cache[imethod.unwrap()] = mega_lookup{{pair, {cls.unwrap(), *method_idx}}};
                return method_idx;
            }
        }
        else
        {
            auto lookup = std::get<mega_lookup>(variant);
            if (auto it = lookup.find(cls.unwrap()); it != lookup.end()) {
                return it->second;
            } else {
                auto method_idx = cls.lookup_interface_method(imethod.name());
                if (method_idx) lookup[cls.unwrap()] = *method_idx;
                return method_idx;
            }
        }
    } else {
        auto method_idx = cls.lookup_interface_method(imethod.name());
        if (method_idx) this->interface_method_cache[imethod.unwrap()] = inlined_lookup{cls.unwrap(), *method_idx};
        return method_idx;
    }
}