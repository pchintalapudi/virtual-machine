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

std::uint32_t young_heap::survival_count(objects::base_object obj)
{
    return this->dead_survivor_boundary > this->live_survivor_boundary ? obj.unwrap() < this->live_survivor_boundary ? obj.tail_data() : 0 : obj.unwrap() < this->live_survivor_boundary ? 0 : obj.tail_data();
}

std::optional<oops::objects::base_object> young_heap::gc_forwarded(objects::base_object obj)
{
    if (obj.marked()) {
        return obj.get_clazz().unwrap();
    } else {
        return {};
    }
}

std::pair<std::optional<oops::objects::base_object>, bool> young_heap::gc_save_young(objects::base_object obj)
{
    if (auto forwarded = this->gc_forwarded(obj))
    {
        return {forwarded, static_cast<std::uintptr_t>(forwarded->unwrap() - this->real_base) > static_cast<std::uintptr_t>(this->real_cap - this->real_base)};
    }
    auto survival_count = this->survival_count(obj);
    auto size = obj.get_clazz().object_malloc_required_size();
    //If survivor space can't host this object or it's old spill it to old generation
    if (static_cast<std::uintptr_t>(std::abs(this->dead_survivor_boundary - this->write_head)) < size + sizeof(std::uint32_t) or this->survival_count(obj) == this->max_young_gc_cycles)
        return {{}, false};
    if (this->live_survivor_boundary < this->dead_survivor_boundary)
    {
        //Grow down to create new heap
        utils::pun_write(this->write_head - sizeof(std::uint32_t) * 2, size64to32(size));
        this->write_head -= size;
        std::memcpy(this->write_head, obj.unwrap(), size - sizeof(std::uint32_t) * 2);
        utils::pun_write(this->write_head - sizeof(std::uint32_t), static_cast<std::uint32_t>(survival_count + 1));
        utils::pun_write(obj.unwrap(), utils::pun_reinterpret<char*>(utils::pun_reinterpret<std::uintptr_t>(this->write_head) | 0b10));
        return {this->write_head, true};
    }
    else
    {
        //Grow up to create new heap
        std::memcpy(this->write_head, obj.unwrap(), size - sizeof(std::uint32_t) * 2);
        utils::pun_write(obj.unwrap(), utils::pun_reinterpret<char*>(utils::pun_reinterpret<std::uintptr_t>(this->write_head) | 0b10));
        utils::pun_write(this->write_head - sizeof(std::uint32_t), static_cast<std::uint32_t>(survival_count + 1));
        objects::object saved(this->write_head);
        this->write_head += size;
        utils::pun_write(this->write_head - sizeof(std::uint32_t) * 2, size64to32(size));
        return {saved, true};
    }
}

oops::memory::young_heap::walker &young_heap::walker::operator++()
{
    if (this->up)
    {
        this->current += objects::base_object(this->current).get_clazz().object_malloc_required_size();
    }
    else
    {
        this->current -= memory::size32to64(utils::pun_read<std::uint32_t>(this->current - sizeof(std::uint32_t) * 2));
    }
    return *this;
}

oops::memory::young_heap::walker &young_heap::walker::operator--()
{
    if (!this->up)
    {
        this->current += objects::base_object(this->current).get_clazz().object_malloc_required_size();
    }
    else
    {
        this->current -= memory::size32to64(utils::pun_read<std::uint32_t>(this->current - sizeof(std::uint32_t) * 2));
    }
    return *this;
}