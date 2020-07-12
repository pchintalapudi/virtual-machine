#include "young_heap.h"
#include "memutils.h"

using namespace oops::memory;

std::optional<oops::objects::object> young_heap::allocate_object(oops::objects::clazz cls)
{
    auto size = cls.object_malloc_required_size() - sizeof(std::uint32_t) * 2;
    if (static_cast<std::uint64_t>(std::abs(this->dead_survivor_boundary - this->write_head)) < size)
        return {};
    if (this->dead_survivor_boundary > this->live_survivor_boundary)
    {
        utils::pun_write(this->write_head, cls.unwrap());
        objects::object obj(this->write_head);
        this->write_head += size;
        return obj;
    }
    else
    {
        this->write_head -= size;
        utils::pun_write(this->write_head, cls.unwrap());
        return objects::object(this->write_head);
    }
}

std::optional<oops::objects::array> young_heap::allocate_array(oops::objects::clazz acls, std::uint64_t asize)
{
    if (static_cast<std::uint64_t>(std::abs(this->dead_survivor_boundary - this->write_head)) < asize)
        return {};
    if (this->dead_survivor_boundary > this->live_survivor_boundary)
    {
        utils::pun_write(this->write_head, acls.unwrap());
        objects::array array(this->write_head);
        this->write_head += asize;
        return array;
    }
    else
    {
        this->write_head -= asize;
        utils::pun_write(this->write_head, acls.unwrap());
        return objects::array(this->write_head);
    }
}