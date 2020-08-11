#ifndef PLATFORM_SPECIFIC_DLLS
#define PLATFORM_SPECIFIC_DLLS

namespace oops
{
    namespace objects
    {
        class clazz;
        class method;
    } // namespace objects

    namespace virtual_machine
    {
        class result;
        class virtual_machine;
    } // namespace virtual_machine
    namespace platform
    {
        virtual_machine::result invoke_native(virtual_machine::virtual_machine* vm, objects::clazz cls, objects::method method);
    }
} // namespace oops

#endif /* PLATFORM_SPECIFIC_DLLS */
