#include "young_heap.h"
#include "memutils.h"
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "windows.h"

using namespace oops::memory;
namespace
{
    char *commit(char *start, char *max, std::size_t page_size, std::size_t page_count)
    {
        if (start == max)
        {
            return nullptr;
        }
        else
        {
            if (VirtualAlloc(start, std::min(page_size * page_count, static_cast<std::size_t>(max - start)), MEM_COMMIT, PAGE_READWRITE))
            {
                return start + page_size * page_count;
            }
            else
            {
                return nullptr;
            }
        }
    }
} // namespace

bool eden_heap::grow(std::size_t page_count)
{
    auto ret = ::commit(this->committed, this->end, this->page_size, page_count);
    if (ret)
    {
        this->committed = ret;
        return true;
    }
    else
    {
        return false;
    }
}

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
    VirtualFree(this->base, 0, MEM_DECOMMIT);
}

std::optional<oops::objects::object> young_heap::allocate_object(objects::clazz cls)
{
    return this->eden.allocate_object(cls);
}

std::optional<oops::objects::array> young_heap::allocate_array(oops::objects::clazz acls, uint64_t memory_size)
{
    return this->eden.allocate_array(acls, memory_size);
}

#endif