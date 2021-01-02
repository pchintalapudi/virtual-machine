#ifndef OOPS_CLASSLOADER_LOADER_ITERATORS_H
#define OOPS_CLASSLOADER_LOADER_ITERATORS_H

#include <cstdint>

#include "class_file_io.h"

namespace oops {
namespace classloading {
class class_file_reader;
class raw_string;
template <typename class_iterator>
class class_iterable {
 private:
  class_iterator start, finish;

 public:
  class_iterable(class_iterator start, class_iterator finish)
      : start(start), finish(finish) {}
  class_iterator begin() { return start; }

  class_iterator end() { return finish; }

  class_iterable slice(std::uint32_t start, std::uint32_t end) {
    return class_iterable(this->start + start, this->start + end);
  }
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
  bool operator!=(const derived &other) const {
    return !(*static_cast<const derived *>(this) == other);
  }
  friend auto operator+(std::uint32_t n, const derived &it) { return it + n; }
};

class class_reference_iterator
    : public class_iterator<class_reference_iterator> {
 private:
  class_file_reader *reader;
  std::uint32_t index;

 public:
  class_reference_iterator(class_file_reader *reader, std::uint32_t index)
      : reader(reader), index(index) {}
  raw_string operator*();

  class_reference_iterator &operator++();
  class_reference_iterator operator+(std::uint32_t n) const;
  bool operator==(const class_reference_iterator &other) const;
};

class instance_field_reference_iterator
    : public class_iterator<instance_field_reference_iterator> {
 private:
  class_file_reader *reader;
  std::uint32_t index;

 public:
  struct field {
    std::uint8_t metadata;
    raw_string name;
  };
  field operator*();

  instance_field_reference_iterator &operator++();
  instance_field_reference_iterator operator+(std::uint32_t n) const;
  bool operator==(const instance_field_reference_iterator &other) const;
};

class static_field_reference_iterator
    : public class_iterator<static_field_reference_iterator> {
 private:
  class_file_reader *reader;
  std::uint32_t index;

 public:
  struct field {
    std::uint8_t metadata;
    raw_string name;
  };
  field operator*();

  static_field_reference_iterator &operator++();
  static_field_reference_iterator operator+(std::uint32_t n) const;
  bool operator==(const static_field_reference_iterator &other) const;
};

class import_iterator : public class_iterator<import_iterator> {
 private:
  class_file_reader *reader;
  std::uint32_t index;

 public:
  import_iterator &operator++() const;
  import_iterator operator+(std::uint32_t n) const;
  bool operator==(const import_iterator &other) const;
};

class method_iterator : public class_iterator<method_iterator> {
 private:
  class_file_reader *reader;
  std::uint32_t index;

 public:
  struct method;
  method operator*();

  method_iterator &operator++();
  method_iterator operator+(std::uint32_t n) const;
  bool operator==(const method_iterator &other) const;
};
}  // namespace classloading
}  // namespace oops

#endif