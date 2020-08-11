#include "files.h"

using namespace oops::platform;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "windows.h"

std::optional<file_mapping> oops::platform::open_class_file_mapping(oops::utils::ostring name)
{
    std::string lpcstr = normalize_file_name(name);
    void *file_handle = CreateFile(lpcstr.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        return {};
    }
    void *file_map_handle = CreateFileMapping(file_handle, NULL, PAGE_READONLY, 0, 0, lpcstr.c_str());
    if (file_map_handle == NULL)
    {
        CloseHandle(file_handle);
        return {};
    }
    void *mmap_handle = MapViewOfFile(file_map_handle, FILE_MAP_READ, 0, 0, 0);
    if (mmap_handle == NULL)
    {
        CloseHandle(file_map_handle);
        CloseHandle(file_handle);
        return {};
    }
    return {{static_cast<char *>(mmap_handle), file_map_handle, file_handle}};
}

std::string oops::platform::normalize_file_name(oops::utils::ostring name)
{
    constexpr const char *bootstrap = "oops";
    std::string lpcstr;
    if (name.length() - std::strlen(".coops") > std::strlen(bootstrap) && std::memcmp(bootstrap, name.unwrap(), std::strlen(bootstrap)))
    {
        lpcstr += platform::get_executable_path();
    }
    else
    {
        lpcstr += platform::get_working_path();
    }
    lpcstr.reserve(lpcstr.length() + name.length() + sizeof(".coops"));
    for (char c : name)
    {
        if (c == '.')
        {
            lpcstr += '\\';
        }
        else
        {
            lpcstr += c;
        }
    }
    lpcstr += ".coops";
    return lpcstr;
}

void oops::platform::close_file_mapping(file_mapping fm)
{
    UnmapViewOfFile(fm.mmapped_file);
    CloseHandle(fm._file_map_handle);
    CloseHandle(fm._file_handle);
}

const char *oops::platform::get_working_path()
{
    return ".\\";
}

const char *oops::platform::get_executable_path()
{
    static char *executable_path = nullptr;
    if (executable_path)
        return executable_path;
    std::size_t executable_size = 8;
    std::size_t string_size = 0;
    do
    {
        char *new_path = static_cast<char *>(std::realloc(executable_path, executable_size <<= 1));
        if (new_path == nullptr)
        {
            std::free(executable_path);
            return executable_path = nullptr;
        }
        string_size = GetModuleFileName(NULL, new_path, executable_size);
    } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);
    if (string_size == 0)
    {
        std::free(executable_path);
        return executable_path = nullptr;
    }
    char *new_path = static_cast<char *>(std::realloc(executable_path, string_size + 1));
    if (new_path)
    {
        executable_path = new_path;
    }
    return executable_path;
}

#endif