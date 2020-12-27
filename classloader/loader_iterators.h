#ifndef OOPS_CLASSLOADER_LOADER_ITERATORS_H
#define OOPS_CLASSLOADER_LOADER_ITERATORS_H

#include <cstdint>

namespace oops {
namespace classloading {
class raw_string;
template <typename class_iterator>
class class_iterable {
 private:
  class_iterator start, finish;

 public:
  class_iterator begin() { return start; }

  class_iterator end() { return end; }
};

template <typename derived>
class class_iterator {
 public:
  derived operator++(int) {
    auto self = static_cast<derived *>(this);
    auto post = *self;
    ++*self;
    return post;
  }
  bool operator!=(const derived &other) {
    return !(*static_cast<derived *>(this) == other);
  }
};

class class_reference_iterator
    : public class_iterator<class_reference_iterator> {
 private:
 public:
  raw_string operator*();

  class_reference_iterator &operator++();
  bool operator==(const class_reference_iterator &other);
};

class instance_field_reference_iterator
    : public class_iterator<instance_field_reference_iterator> {
 private:
 public:
  struct field {
    std::uint8_t metadata;
    std::uint32_t offset;
  };
  field operator*();

  instance_field_reference_iterator &operator++();
  bool operator==(const instance_field_reference_iterator &other);
};

class static_field_reference_iterator
    : public class_iterator<static_field_reference_iterator> {
 private:
 public:
  struct field {
    std::uint8_t metadata;
    std::uint32_t offset;
  };
  field operator*();

  static_field_reference_iterator &operator++();
  bool operator==(const static_field_reference_iterator &other);
};

class method_iterator : public class_iterator<method_iterator> {
 private:
 public:
  struct method;
  method operator*();

  method_iterator &operator++();
  bool operator==(const method_iterator &other);
};
}  // namespace classloading
}  // namespace oops

#endif