#ifndef OOPS_CLASSES_OBJECT_H
#define OOPS_CLASSES_OBJECT_H

#include <optional>

#include "../memory/byteblock.h"

namespace oops {
    namespace classes {

        class array;
        class object;
        class clazz;
        enum class datatype;

        class base_object {
            private:
            memory::byteblock<> data;
            public:

            base_object(void* obj);

            bool is_null() const;

            clazz get_class() const;

            bool is_array() const;

            array as_array() const;
            object as_object() const;

            void* get_raw() const;

            bool operator==(const base_object& other) const {
                return this->data == other.data;
            }

            bool operator<(const base_object& other) const {
                return this->data < other.data;
            }
        };

        class array {
            private:
            memory::byteblock<> data;

            template<typename out_t>
            out_t read(std::int32_t index) {
                return data.read<out_t>(static_cast<std::int64_t>(index) * sizeof(out_t) + sizeof(std::int32_t) * 2);
            }
            template<typename in_t>
            void write(std::int32_t index, in_t data) {
                this->data.write(static_cast<std::int64_t>(index) * sizeof(in_t) + sizeof(std::int32_t) * 2, data);
            }
            public:

            array(void* arr);

            datatype element_type() const;

            std::int32_t length() const;

            template<typename out_t>
            std::optional<out_t> get(std::int32_t index) {
                //TODO check type
                if constexpr (std::is_same_v<out_t, base_object>) {
                    void* obj = this->read<void*>(index);
                    return base_object(obj);
                } else {
                    return this->read<out_t>(index);
                }
            }

            template<typename in_t>
            bool set(std::int32_t index, in_t value) {
                //TODO check type
                if constexpr (std::is_same_v<in_t, base_object>) {
                    void* obj = value.get_raw();
                    this->write(index, obj);
                    return true;
                } else {
                    this->write(index, value);
                    return true;
                }
            }

            base_object to_base_object() const;

            bool operator==(const array& other) const {
                return this->data == other.data;
            }

            bool operator<(const array& other) const {
                return this->data < other.data;
            }
        };

        class object {
            private:
            memory::byteblock<> data;
            public:

            object(void* obj);

            base_object to_base_object() const;

            bool operator==(const object& other) const {
                return this->data == other.data;
            }

            bool operator<(const object& other) const {
                return this->data < other.data;
            }
        };

    }
}

#endif /* CLASSES_OBJECT */
