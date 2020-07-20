#ifndef MEMORY_YOUNG_HEAP
#define MEMORY_YOUNG_HEAP

#include "../objects/objects.h"

namespace oops
{
    namespace memory
    {
        class heap;

        class young_heap
        {
        private:
            friend class heap;
            char *real_base, *real_cap;
            char *live_survivor_boundary, *dead_survivor_boundary;
            char *write_head;
            std::uint32_t max_young_gc_cycles;

            double requested_free_ratio = 0.4;
            std::uint64_t allocation_granularity;
            std::uint64_t survivor_space_size;

            bool prep_for_gc();

            void finish_young_gc();

            std::uint32_t survival_count(objects::base_object);
            std::pair<std::optional<objects::base_object>, bool> gc_save_young(objects::base_object obj);

            std::optional<objects::base_object> gc_forwarded(objects::base_object);

            class walker
            {
            private:

                friend class young_heap;

                char *current;
                const bool up;

                template<typename proxy_type>
                class arrow_proxy {
                    private:
                    proxy_type pt;
                    public:
                    arrow_proxy(proxy_type pt) : pt(pt) {}

                    proxy_type* operator->() {
                        return &this->pt;
                    }
                };

                public:

                walker(char* base, bool up) : current(base), up(up) {}

                walker &operator++();

                walker &operator--();

                objects::base_object operator*() {
                    return objects::base_object(this->current);
                }

                arrow_proxy<objects::base_object> operator->() {
                    return **this;
                }

                bool operator<(const walker& other) const {
                    return this->current < other.current;
                }

                bool operator==(const walker& other) const {
                    return this->current == other.current;
                }

                bool operator>(const walker& other) const {
                    return this->current > other.current;
                }

                bool operator<=(const walker& other) const {
                    return this->current <= other.current;
                }

                bool operator>=(const walker& other) const {
                    return this->current >= other.current;
                }

                bool operator!=(const walker& other) const {
                    return this->current != other.current;
                }
            };
            walker begin() {
                return this->live_survivor_boundary < this->dead_survivor_boundary ? walker{this->write_head, false} : walker{this->real_base, true};
            }

            auto end() {
                return this->live_survivor_boundary < this->dead_survivor_boundary ? walker{this->real_cap, false} : walker{this->write_head, true};
            }
        public:
            std::optional<objects::object> allocate_object(objects::clazz cls);

            std::optional<objects::array> allocate_array(objects::clazz acls, std::uint64_t memory_size);
        };
    } // namespace memory
} // namespace oops

#endif /* MEMORY_YOUNG_HEAP */
