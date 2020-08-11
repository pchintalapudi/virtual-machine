#include "dlls.h"

#include "../objects/objects.h"
#include "../virtual_machine/vm.h"
#include "../native/api.h"
#include "./files.h"

using namespace oops::platform;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#include "windows.h"

oops::virtual_machine::result oops::platform::invoke_native(oops::virtual_machine::virtual_machine *vm, oops::objects::clazz cls, oops::objects::method method)
{
    std::string dll_name = platform::normalize_file_name(cls.name());
    dll_name.erase(dll_name.end() - strlen("coops"));
    dll_name += "dll";
    HMODULE dll = LoadLibrary(dll_name.c_str());
    std::string method_name;
    auto name = method.name();
    method_name.reserve(name.length() + 1);
    std::copy(name.begin(), name.end(), std::back_inserter(method_name));
    if (dll)
    {
        FARPROC native = GetProcAddress(dll, method_name.c_str());
        if (native)
        {
            auto result = reinterpret_cast<native_function>(reinterpret_cast<void*>(native))({static_cast<void *>(vm)});
            switch (result.type)
            {
            case static_cast<unsigned>(objects::field::type::INT):
                return virtual_machine::result(result.int_value, result.status);
            case static_cast<unsigned>(objects::field::type::LONG):
                return virtual_machine::result(result.long_value, result.status);
            case static_cast<unsigned>(objects::field::type::FLOAT):
                return virtual_machine::result(result.float_value, result.status);
            case static_cast<unsigned>(objects::field::type::DOUBLE):
                return virtual_machine::result(result.double_value, result.status);
            case static_cast<unsigned>(objects::field::type::OBJECT):
                return virtual_machine::result(objects::base_object(static_cast<char *>(result.object_value)), result.status);
            default:
                break;
            }
        }
    }
    return virtual_machine::result(objects::base_object(nullptr), -1);
}

#endif