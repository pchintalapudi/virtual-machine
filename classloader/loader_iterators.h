#ifndef OOPS_CLASSLOADER_LOADER_ITERATORS_H
#define OOPS_CLASSLOADER_LOADER_ITERATORS_H

#include <cstdint>
#include <type_traits>

#include "../classes/datatypes.h"
#include "class_file_io.h"

namespace oops {
namespace classloading {
class raw_string;

template <typename class_iterator>
class class_iterable {
 private:
  class_iterator start, finish;

  
  template<typename cit = class_iterator>
  std::enable_if_t<
      std::is_same_v<std::random_access_iterator_tag,
                     typename std::iterator_traits<cit>::iterator_category>,
      class_iterable>
  slice_impl(std::uint32_t start, std::uint32_t end) {
    return class_iterable(this->start + start, this->start + end);
  }

 public:
  class_iterable(class_iterator start, class_iterator finish)
      : start(start), finish(finish) {}
  class_iterator begin() { return start; }

  class_iterator end() { return finish; }

  class_iterable slice(std::uint32_t start, std::uint32_t end) {
      return slice_impl(start, end);
  }
};

template <typename derived>
class comparable {
 public:
  bool operator>(const derived &other) const {
    return other < *static_cast<const derived *>(this);
  }
  bool operator>=(const derived &other) const {
    return other <= *static_cast<const derived *>(this);
  }
  bool operator<=(const derived &other) const {
    return other == *static_cast<derived *>(this) ||
           other < *static_cast<const derived *>(this);
  }
  bool operator!=(const derived &other) const {
    return !(other == *static_cast<const derived *>(this));
  }
};
#define self                                                                 \
  static_cast<std::conditional_t<                                            \
      std::is_const_v<std::remove_reference_t<decltype(*this)>>, const It *, \
      It *>>(this)
template <typename derived, typename ref, std::int64_t stride_length,
          typename datatype>
class class_random_access_iterator : public comparable<derived> {
 public:
  typedef std::int64_t difference_type;
  typedef derived It;
  typedef std::random_access_iterator_tag iterator_category;
  typedef ref reference;
  typedef reference value_type;
  typedef void pointer;

 protected:
  datatype *reader;
  difference_type index;

 public:
  class_random_access_iterator(datatype *reader, difference_type index)
      : reader(reader), index(index) {}

  It &operator+=(difference_type n) {
    this->index += n * stride_length;
    return *self;
  }
  bool operator==(const It &other) const { return this->index == other.index; }
  bool operator<(const It &other) const { return this->index < other.index; }
  It &operator++() { return *self += 1; }
  It operator++(int) {
    auto post = *self;
    ++*self;
    return post;
  }
  It &operator--() { return *self -= 1; }
  It &operator-=(difference_type n) { return *self += -n; }
  It operator--(int) {
    auto post = *self;
    --*self;
    return post;
  }
  It operator-(difference_type n) const {
    auto post = *self;
    post -= n;
    return post;
  }
  difference_type operator-(const It &other) const {
    return (this->index - other.index) / stride_length;
  }
  It operator+(difference_type n) const {
    It post = *self;
    post += n;
    return post;
  }
  friend auto operator+(difference_type n, const It &it) { return it + n; }

  reference operator[](difference_type n) {
    *self += n;
    reference out = **self;
    *self -= n;
    return out;
  }
};
#undef self

struct class_reference {
  raw_string name;
  std::uint32_t string_idx;
};

class class_reference_iterator : public class_random_access_iterator<
                                     class_reference_iterator, class_reference,
                                     sizeof(std::uint32_t), class_file_reader> {
 public:
  typedef class_random_access_iterator<class_reference_iterator,
                                       class_reference, sizeof(std::uint32_t),
                                       class_file_reader>
      super;
  using super::class_random_access_iterator;
  reference operator*();
};

struct field : public comparable<field> {
  raw_string name;
  std::uint32_t string_idx;
  std::uint32_t data_idx;
  classes::field_type field_type;
  std::uint8_t data_type;
  bool operator==(const field &other) const;
  bool operator<(const field &other) const;
};

class instance_field_reference_iterator
    : public class_random_access_iterator<instance_field_reference_iterator,
                                          field, sizeof(std::uint32_t),
                                          class_file_reader> {
 public:
  typedef class_random_access_iterator<instance_field_reference_iterator, field,
                                       sizeof(std::uint32_t), class_file_reader>
      super;
  using super::class_random_access_iterator;
  reference operator*();
};

class static_field_reference_iterator
    : public class_random_access_iterator<static_field_reference_iterator,
                                          field, sizeof(std::uint32_t),
                                          class_file_reader> {
 public:
  typedef class_random_access_iterator<static_field_reference_iterator, field,
                                       sizeof(std::uint32_t), class_file_reader>
      super;
  using super::class_random_access_iterator;
  reference operator*();
};

struct import_reference {
  classes::field_type type;
  std::uint8_t subtype;
  raw_string name;
  std::uint32_t string_idx;
  std::uint32_t class_reference;
};

class import_iterator
    : public class_random_access_iterator<import_iterator, import_reference,
                                          sizeof(std::uint32_t) * 2,
                                          class_file_reader> {
 public:
  typedef class_random_access_iterator<import_iterator, import_reference,
                                       sizeof(std::uint32_t) * 2,
                                       class_file_reader>
      super;
  using super::class_random_access_iterator;
  reference operator*() const;
};

class method_iterator : public comparable<method_iterator> {
 public:
  typedef std::int64_t difference_type;
  typedef field reference;
  typedef method_iterator It;
  typedef field value_type;
  typedef void pointer;
  typedef std::forward_iterator_tag iterator_category;

 private:
  class_file_reader *reader;
  difference_type index;

 public:
  method_iterator(class_file_reader *reader, difference_type index)
      : reader(reader), index(index) {}
  field operator*();
  bool operator==(const method_iterator &other) const {
    return this->index == other.index;
  }
  bool operator<(const method_iterator &other) const {
    return this->index < other.index;
  }
  method_iterator &operator++();
  method_iterator operator++(int) {
    auto post = *this;
    ++*this;
    return post;
  }
};
}  // namespace classloading
}  // namespace oops

#endif