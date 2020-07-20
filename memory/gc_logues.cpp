#include "young_heap.h"
#include "old_heap.h"
#include "heap.h"

#include "../platform_specific/memory.h"

using namespace oops::memory;

void old_heap::finish_old_gc(std::uint64_t free)
{
    std::uint64_t required_free = (this->head - this->base) * this->requested_free_ratio + 1;
    if (free < required_free)
    {
        auto required_increase = required_free - free;
        auto requested_commit_size = required_increase / this->allocation_granularity + (required_increase % this->allocation_granularity > 0);
        auto actual_commit_size = std::min(static_cast<std::uint64_t>(this->cap - this->head), requested_commit_size);
        if (actual_commit_size > 0)
        {
            if (platform::commit(this->head, actual_commit_size))
            {
                this->head += actual_commit_size;
            }
        }
    }
    //We're never going to give back memory to the OS cause I'm lazy
}

void old_heap::prep_for_gc()
{
    //Do nothing
}

void young_heap::finish_young_gc()
{
    if (this->live_survivor_boundary < this->dead_survivor_boundary)
    {
        //Was growing up, now grow down
        platform::decommit(this->real_base, this->dead_survivor_boundary - this->real_base);
        std::uint64_t required_free = (this->dead_survivor_boundary - this->real_base) * this->requested_free_ratio + 1;
        char *normalized_write_head = this->write_head - (this->write_head - this->real_base) % this->allocation_granularity;
        std::uint64_t free = std::min(this->dead_survivor_boundary - this->real_base, this->real_cap - this->dead_survivor_boundary);
        this->live_survivor_boundary = this->write_head;
        if (free < required_free)
        {
            auto required_increase = required_free - free;
            auto requested_commit_size = required_increase / this->allocation_granularity + (required_increase % this->allocation_granularity > 0);
            auto actual_commit_size = std::min(requested_commit_size, static_cast<std::uint64_t>(normalized_write_head - this->real_base - this->survivor_space_size));
            if (platform::commit(normalized_write_head - actual_commit_size, actual_commit_size))
            {
                this->dead_survivor_boundary = normalized_write_head - actual_commit_size;
            }
            else
            {
                this->dead_survivor_boundary = normalized_write_head;
            }
        }
        else
        {
            this->dead_survivor_boundary = normalized_write_head;
        }
    }
    else
    {
        //Was growing down, now grow up
        platform::decommit(this->dead_survivor_boundary, this->real_cap - this->dead_survivor_boundary);
        std::uint64_t required_free = (this->real_cap - this->dead_survivor_boundary) * this->requested_free_ratio + 1;
        char *normalized_write_head = this->write_head + (this->allocation_granularity - (this->write_head - this->real_base) % this->allocation_granularity);
        std::uint64_t free = std::min(this->real_cap - this->dead_survivor_boundary, this->dead_survivor_boundary - this->real_base);
        this->live_survivor_boundary = this->write_head;
        if (free < required_free)
        {
            auto required_increase = required_free - free;
            auto requested_commit_size = required_increase / this->allocation_granularity + (required_increase % this->allocation_granularity > 0);
            auto actual_commit_size = std::min(requested_commit_size, static_cast<std::uint64_t>(this->real_cap - normalized_write_head - this->survivor_space_size));
            if (platform::commit(normalized_write_head, actual_commit_size))
            {
                this->dead_survivor_boundary = normalized_write_head + actual_commit_size;
            }
            else
            {
                this->dead_survivor_boundary = normalized_write_head;
            }
        }
        else
        {
            this->dead_survivor_boundary = normalized_write_head;
        }
    }
}

bool young_heap::prep_for_gc()
{
    if (this->live_survivor_boundary < this->dead_survivor_boundary)
    {
        auto amount = this->dead_survivor_boundary - this->real_base;
        if (!platform::commit(this->real_cap - amount, amount))
        {
            return false;
        }
        this->write_head = this->real_cap - sizeof(std::uint64_t);
        return true;
    }
    else
    {
        auto amount = this->real_cap - this->dead_survivor_boundary;
        if (!platform::commit(this->real_base, amount))
        {
            return false;
        }
        this->write_head = this->real_base + sizeof(std::uint64_t);
        return true;
    }
}

bool heap::prep_for_young_gc()
{
    if (this->young_generation.live_survivor_boundary < this->young_generation.dead_survivor_boundary)
    {
        auto precommitted = this->young_generation.real_cap - this->young_generation.dead_survivor_boundary - sizeof(std::uint64_t);
        if (precommitted < static_cast<std::uint64_t>(this->young_generation.dead_survivor_boundary - this->young_generation.real_base))
        {
            std::uint64_t difference = this->young_generation.dead_survivor_boundary - this->young_generation.real_base - precommitted;
            auto guaranteed_object_count = difference / this->max_young_object_size + (difference % this->max_young_object_size > 0);
            if (!this->old_generation.guarantee(guaranteed_object_count, this->max_young_object_size))
            {
                return false;
            }
            if (!this->young_generation.prep_for_gc())
            {
                return false;
            }
        }
    }
    else
    {
        auto precommitted = this->young_generation.dead_survivor_boundary - this->young_generation.real_base - sizeof(std::uint64_t);
        if (precommitted < static_cast<std::uint64_t>(this->young_generation.real_cap - this->young_generation.dead_survivor_boundary))
        {
            std::uint64_t difference = this->young_generation.real_cap - this->young_generation.dead_survivor_boundary - precommitted;
            auto guaranteed_object_count = difference / this->max_young_object_size + (difference % this->max_young_object_size > 0);
            if (!this->old_generation.guarantee(guaranteed_object_count, this->max_young_object_size))
            {
                return false;
            }
            if (!this->young_generation.prep_for_gc())
            {
                return false;
            }
        }
    }
    return true;
}

void heap::finish_young_gc() {
    return this->young_generation.finish_young_gc();
}

void heap::prep_for_old_gc() {
    this->old_generation.prep_for_gc();
}

bool old_heap::guarantee(std::uint64_t, std::uint64_t) {
    return true; //TODO actually implement this
}