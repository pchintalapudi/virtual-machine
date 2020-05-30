#include "objects.h"
#include "../punning/puns.h"

using namespace oops::objects;

namespace
{
    inline bool field_name_bound(const oops::objects::field &field, std::uint32_t name_length)
    {
        return field.get_name_length() < name_length;
    }
} // namespace

const field *clazz::lookup_field(char* clazz, const char *name, std::uint32_t name_length)
{
    std::uintptr_t mcount_offset = sizeof(std::uint32_t);
    PUN(std::uint32_t, method_count, clazz + mcount_offset);
    std::uintptr_t count_offset = mcount_offset + sizeof(std::uint32_t) * 3 + sizeof(char *) * method_count + sizeof(char *), start_offset = count_offset + sizeof(uint32_t) * 2;
    PUN(std::uint32_t, field_count, clazz + count_offset);
    PUN(field *, start, clazz + start_offset);
    field *end = start + field_count;
    for (field *candidate = std::lower_bound(start, end, name_length, ::field_name_bound); candidate < end && candidate->get_name_length() == name_length; candidate++)
    {
        const char *cname = candidate->get_name();
        if (cname == name || memcmp(name, cname, name_length) == 0)
            return candidate;
    }
    return nullptr;
}

void construct_object(char *aligned_8_byte_location, char* class_ptr)
{
    std::memcpy(aligned_8_byte_location, class_ptr + sizeof(uint32_t) * 2, sizeof(uint32_t));
    std::memcpy(aligned_8_byte_location + sizeof(uint32_t), class_ptr + sizeof(uint32_t) * 3, sizeof(uint32_t));
    uint32_t size;
    std::memcpy(&size, class_ptr + sizeof(uint32_t) * 3, sizeof(uint32_t));
    std::memcpy(aligned_8_byte_location + sizeof(uint32_t) * 2, &class_ptr, sizeof(char *));
    std::memset(aligned_8_byte_location + sizeof(uint32_t) * 2 + sizeof(char *), 0, size - sizeof(char *) - 2 * sizeof(std::uint32_t));
}

void clazz::construct_class(char *aligned_8_byte_location, class_def &definition)
{
    std::memcpy(aligned_8_byte_location, &definition.class_size, sizeof(std::uint32_t));
    std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), &definition.method_count, sizeof(std::uint32_t));
    std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), &definition.object_size, sizeof(std::uint32_t));
    std::uint32_t method_count;
    std::memcpy(&method_count, &definition.method_count, sizeof(std::uint32_t));
    std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), &definition.handle_count, sizeof(std::uint32_t));
    std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), definition.methods, sizeof(char *) * method_count);
    std::memcpy(aligned_8_byte_location += sizeof(char *) * method_count, &definition.class_object, sizeof(char *));
    std::memcpy(aligned_8_byte_location += sizeof(char *), &definition.field_count, sizeof(std::uint32_t));
    std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), &definition.name_length, sizeof(std::uint32_t));
    std::memcpy(aligned_8_byte_location += sizeof(std::uint32_t), &definition.name, sizeof(char *));
    definition.fields = nullptr;
}

void clazz::destruct_class(char *aligned_8_byte_location)
{
    uint32_t method_count;
    std::memcpy(&method_count, aligned_8_byte_location + sizeof(std::uint32_t), sizeof(std::uint32_t));
    field *fields;
    std::memcpy(&fields, aligned_8_byte_location + sizeof(std::uint32_t) * 4 + method_count * sizeof(char *) + sizeof(char *) + sizeof(std::uint32_t) * 2, sizeof(field *));
    delete[] fields;
}