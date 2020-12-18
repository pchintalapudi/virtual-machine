#include "class.h"

using namespace oops::classes;

namespace {
    enum CLASS_HEADER_BYTE_OFFSET {
        SUPERCLASS = 0,
        OBJECT_SIZE = SUPERCLASS + 8,
        IPOINTER = OBJECT_SIZE + 4,
        IDOUBLE = IPOINTER + 2,
        ILONG = IDOUBLE + 2,
        IFLOAT = ILONG + 2,
        IINT = IFLOAT + 2,
        ISHORT = IINT + 2,
        IBYTE = ISHORT + 2,
        SPOINTER = IBYTE + 2,
        SDOUBLE = SPOINTER + 2,
        SLONG = SDOUBLE + 2,
        SFLOAT = SLONG + 2,
        SINT = SFLOAT + 2,
        SSHORT = SINT + 2,
        SBYTE = SSHORT + 2,
        VIRTUAL_METHOD_COUNT = SBYTE + 2,
        REFLECTION_STRING_COUNT = VIRTUAL_METHOD_COUNT + 4,
        BYTECODE_SIZE = REFLECTION_STRING_COUNT + 4,
        STRING_POOL_COUNT = BYTECODE_SIZE + 4
    };
}

clazz::clazz(void* cls) {
    class_data.initialize(cls);
}

std::optional<oops::classes::clazz> clazz::superclass() const {
    void* superclass = this->class_data.read<void*>(CLASS_HEADER_BYTE_OFFSET::SUPERCLASS);
    if (superclass) {
        return clazz(superclass);
    }
    return {};
}

std::uint32_t clazz::object_size() const {
    return this->class_data.read<std::uint32_t>(CLASS_HEADER_BYTE_OFFSET::OBJECT_SIZE);
}

std::uint16_t clazz::instance_pointer_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::IPOINTER);
}
std::uint16_t clazz::instance_double_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::IDOUBLE);
}
std::uint16_t clazz::instance_long_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::ILONG);
}
std::uint16_t clazz::instance_float_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::IFLOAT);
}
std::uint16_t clazz::instance_int_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::IINT);
}
std::uint16_t clazz::instance_short_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::ISHORT);
}
std::uint16_t clazz::instance_byte_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::IBYTE);
}

std::uint16_t clazz::static_pointer_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::SPOINTER);
}
std::uint16_t clazz::static_double_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::SDOUBLE);
}
std::uint16_t clazz::static_long_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::SLONG);
}
std::uint16_t clazz::static_float_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::SFLOAT);
}
std::uint16_t clazz::static_int_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::SINT);
}
std::uint16_t clazz::static_short_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::SSHORT);
}
std::uint16_t clazz::static_byte_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::SBYTE);
}

std::uint32_t clazz::virtual_method_count() const {
    return this->class_data.read<std::uint32_t>(CLASS_HEADER_BYTE_OFFSET::VIRTUAL_METHOD_COUNT);
}

std::uint32_t clazz::total_reflection_count() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::REFLECTION_STRING_COUNT);
}

std::uint32_t clazz::total_bytecode_size() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::BYTECODE_SIZE);
}

std::uint32_t clazz::total_string_pool_size() const {
    return this->class_data.read<std::uint16_t>(CLASS_HEADER_BYTE_OFFSET::STRING_POOL_COUNT);
}