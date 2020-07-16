#include "heap.h"

using namespace oops::memory;

std::optional<oops::objects::object> heap::allocate_object(oops::objects::clazz cls)
{
    if (cls.object_malloc_required_size() < this->max_young_object_size)
    {
        return this->young_generation.allocate_object(cls);
    }
    else
    {
        return this->old_generation.allocate_object(cls);
    }
}

std::optional<oops::objects::array> heap::allocate_array(oops::objects::clazz acls, std::uint64_t memory_size)
{
    if (memory_size < this->max_young_object_size)
    {
        return this->young_generation.allocate_array(acls, memory_size);
    }
    else
    {
        return this->old_generation.allocate_array(acls, memory_size);
    }
}
void heap::write_barrier(char *dest, char *obj)
{
    if (dest < old_generation.cap and obj >= old_generation.cap)
    {
        forward_references.insert(dest);
    }
    else if (dest > old_generation.cap and obj <= old_generation.cap)
    {
        back_references.insert(obj);
    }
}

std::pair<std::optional<oops::objects::base_object>, heap::location> heap::gc_move_young(oops::objects::base_object obj)
{
    if (auto saved = this->young_generation.gc_save_young(obj); saved.first)
    {
        return {saved.first, saved.second ? location::SURVIVOR : location::FORWARD_TENURED};
    }
    else
    {
        auto size = obj.get_clazz().object_malloc_required_size();
        if (auto old_saved = this->old_generation.allocate_memory(size))
        {
            std::memcpy(*old_saved, obj.unwrap(), size - sizeof(std::uint32_t) * 2);
            return {objects::object(*old_saved), location::TENURED};
        } else {
            return {{}, location::TENURED};
        }
    }
}