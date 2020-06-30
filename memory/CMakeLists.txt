target_sources(oops
PRIVATE
objects.cpp
objects.h
heap.h
heap.cpp
memory_manager.h
stack.h
)

add_subdirectory(smalloc)
add_subdirectory(bumpy)
add_subdirectory(classes)