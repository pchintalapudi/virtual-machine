#include "young_heap.h"
#include "memutils.h"

#include "../platform_specific/memory.h"

using namespace oops::memory;

std::optional<oops::objects::object> eden_heap::allocate_object(objects::clazz cls)
{
    auto size = cls.object_malloc_required_size() - sizeof(sizeof(std::uint64_t));
    if (static_cast<std::uint64_t>(this->committed - this->head) < size)
    {
        return {};
    }
    else
    {
        utils::pun_write(this->head + sizeof(std::uint32_t), cls.unwrap());
        auto obj = oops::objects::object(this->head + sizeof(std::uint32_t));
        this->head += size;
        return {obj};
    }
}
using ftype = oops::objects::field::type;
std::optional<oops::objects::array> eden_heap::allocate_array(oops::objects::clazz acls, uint64_t memory_size) {
    if (static_cast<std::uint64_t>(this->committed - this->head) < memory_size) {
        return {};
    } else {
        utils::pun_write(this->head + sizeof(std::uint32_t), acls.unwrap());
        auto obj = oops::objects::array(this->head + sizeof(std::uint32_t));
        this->head += memory_size;
        return {obj};
    }
}

eden_heap::~eden_heap()
{
    platform::decommit(this->base, this->committed - this->base);
}

std::optional<oops::objects::object> young_heap::allocate_object(objects::clazz cls)
{
    return this->eden.allocate_object(cls);
}

std::optional<oops::objects::array> young_heap::allocate_array(oops::objects::clazz acls, uint64_t memory_size)
{
    return this->eden.allocate_array(acls, memory_size);
}