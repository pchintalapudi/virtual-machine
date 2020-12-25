#include "memory.h"

using namespace oops::platform;

#include "platform_selector.h"

#ifdef OOPS_COMPILE_FOR_WINDOWS

#include <memoryapi.h>
#include <sysinfoapi.h>

std::optional<void*> oops::platform::reserve_virtual_memory(std::size_t amount) {
    auto result = VirtualAlloc(NULL, amount, MEM_RESERVE, PAGE_READWRITE);
    if (result) {
        return result;
    } else {
        return {};
    }
}

std::optional<void*> oops::platform::commit_virtual_memory(void* reserved, std::size_t amount) {
    auto result = VirtualAlloc(reserved, amount, MEM_COMMIT, PAGE_READWRITE);
    if (result) {
        return result;
    } else {
        return {};
    }
}

void oops::platform::decommit_virtual_memory(void* committed, std::size_t amount) {
    VirtualFree(committed, amount, MEM_DECOMMIT);
}

void oops::platform::dereserve_virtual_memory(void* reserved) {
    VirtualFree(reserved, 0, MEM_RELEASE);
}

const system_info& oops::platform::get_system_info() {
    static bool initialized = false;
    static system_info value;
    if (!initialized) {
        SYSTEM_INFO storage;
        GetNativeSystemInfo(&storage);
        value.page_size = storage.dwAllocationGranularity;
        value.processor_count = storage.dwNumberOfProcessors;
    }
    return value;
}

#endif