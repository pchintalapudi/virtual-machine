# Garbage Collection

---

## Strategies

---

### Wasteful Garbage Collection

This strategy is a simple stop-and-copy operation, that skips complex features such as write barriers and complicated allocators. It forms the core strategy of the young generation of generational garbage collection.

### Generational Garbage Collection

This strategy employs 2 generations, a young generation and a tenured generation. The young generation is garbage collected by copying, while the tenured generation is garbage collected in-place by walking the heap. This requires knowledge of the allocation header information, and therefore requires a custom allocator.

## Parallelizability

Concurrent graph traversal is inherently parallelizable, which makes the tenured marking phase faster. Implementing wasteful garbage collection in parallel is not worth the effort due to its exclusive use for testing alone.

## Object Graph Traversal

### Roots

There are a few roots from which all garbage collections will begin.

1. Execution stacks
1. Class pointers
   1. Class static pointers
   1. Class string pool
1. Native object references
   1. Reference counted native objects are atomic for safety

Graph traversal begins with a root pointer, and performs a traversal to mark every node in the graph as visited.
