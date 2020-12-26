#ifndef OOPS_GC_ITERABLE_H
#define OOPS_GC_ITERABLE_H

namespace oops {
namespace gc {
template <typename iterator_t>
class iterable {
 private:
  iterator_t start, finish;

 public:
  iterable(iterator_t start, iterator_t finish) : start(start), finish(finish) {}

  iterator_t begin() { return start; }

  iterator_t end() { return finish; }
};
}  // namespace gc
}  // namespace oops

#endif