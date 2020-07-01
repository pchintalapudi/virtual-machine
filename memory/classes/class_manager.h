#ifndef MEMORY_CLASSES_CLASS_MANAGER_H
#define MEMORY_CLASSES_CLASS_MANAGER_H

#include <cstdint>
#include <unordered_map>
#include <memory>
#include <variant>
#include <vector>

#include "../../utils/ostring.h"
#include "../objects.h"
namespace oops
{
    namespace classes
    {

        class class_manager
        {
        private:
            struct class_hasher
            {
                std::size_t operator()(const oops::objects::clazz &cls) const
                {
                    return std::hash<char *>()(cls.unwrap());
                }

                std::size_t operator()(const oops::objects::method &mtd) const
                {
                    return std::hash<char*>()(mtd.unwrap());
                }
            };
            std::unordered_map<oops::objects::clazz, std::uint32_t, class_hasher> class_indexer;
            std::vector<std::uintptr_t> relations;

            class fast_interface_cache
            {
            private:
                union {
                    std::pair<oops::objects::clazz, oops::objects::method> monomorphic_lookup;
                    std::unordered_map<oops::objects::clazz, oops::objects::method, class_hasher> megamorphic_lookup;
                };
                bool megamorphic;

            public:
                fast_interface_cache(oops::objects::clazz cls, oops::objects::method mtd) : monomorphic_lookup({cls, mtd})
                {
                }

                std::pair<bool, oops::objects::method> operator[](oops::objects::clazz cls)
                {
                    if (this->megamorphic)
                    {
                        auto it = megamorphic_lookup.find(cls);
                        if (it != megamorphic_lookup.end())
                        {
                            return std::make_pair(true, it->second);
                        }
                        else
                        {
                            return std::make_pair(false, oops::objects::method(nullptr));
                        }
                    }
                    else
                    {
                        return this->monomorphic_lookup.first == cls ? std::make_pair(true, this->monomorphic_lookup.second) : std::make_pair(false, oops::objects::method(nullptr));
                    }
                }

                void cache(oops::objects::clazz cls, oops::objects::method mtd)
                {
                    if (!this->megamorphic)
                    {
                        auto pair = this->monomorphic_lookup;
                        this->megamorphic = true;
                        this->monomorphic_lookup.~pair();
                        this->megamorphic_lookup = {pair};
                    }
                    this->megamorphic_lookup.emplace(cls, mtd);
                }

                ~fast_interface_cache()
                {
                    if (this->megamorphic)
                    {
                        this->megamorphic_lookup.~unordered_map();
                    }
                    else
                    {
                        this->monomorphic_lookup.~pair();
                    }
                }
            };

            std::unordered_map<oops::objects::method, fast_interface_cache, class_hasher> interface_lookup_cache;

        public:
            bool instanceof (oops::objects::clazz cls, oops::objects::clazz type);

            std::pair<bool, oops::objects::method> lookup_interface_cache(oops::objects::method imethod, oops::objects::clazz cls)
            {
                auto it = interface_lookup_cache.find(imethod);
                if (it != interface_lookup_cache.end())
                {
                    return it->second[cls];
                }
                else
                {
                    return {false, oops::objects::method(nullptr)};
                }
            }
        };
    } // namespace classes
} // namespace oops

#endif