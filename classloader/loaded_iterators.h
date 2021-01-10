#ifndef OOPS_CLASSLOADER_LOADED_ITERATORS_H
#define OOPS_CLASSLOADER_LOADED_ITERATORS_H

#include "class_file_io.h"
#include "loader_iterators.h"

namespace oops {
namespace classloading {

template <typename payload>
class bdr_reference;

class loaded_class_reference_iterator
    : public class_random_access_iterator<loaded_class_reference_iterator,
                                          bdr_reference<class_reference>,
                                          sizeof(std::uint32_t), class_writer> {
 private:
  friend class bdr_reference<class_reference>;

 public:
  typedef class_random_access_iterator<loaded_class_reference_iterator,
                                       bdr_reference<class_reference>,
                                       sizeof(std::uint32_t), class_writer>
      super;
  using super::class_random_access_iterator;
  reference operator*();
};

template <>
class bdr_reference<class_reference>
    : public comparable<bdr_reference<class_reference>> {
 private:
  loaded_class_reference_iterator *it;

 public:
  bdr_reference(loaded_class_reference_iterator *it) : it(it) {}
  operator class_reference() const;
  bdr_reference &operator=(const class_reference &reference);
  bool operator==(const bdr_reference &other) const {
    return this->it == other.it;
  }
};

class loaded_import_reference_iterator
    : public class_random_access_iterator<loaded_import_reference_iterator,
                                          bdr_reference<import_reference>,
                                          sizeof(std::uint32_t), class_writer> {
 private:
  friend class bdr_reference<import_reference>;

 public:
  typedef class_random_access_iterator<loaded_import_reference_iterator,
                                       bdr_reference<import_reference>,
                                       sizeof(std::uint32_t), class_writer>
      super;
  using super::class_random_access_iterator;
  reference operator*();
};

template <>
class bdr_reference<import_reference>
    : public comparable<bdr_reference<import_reference>> {
 private:
  loaded_import_reference_iterator *it;

 public:
  bdr_reference(loaded_import_reference_iterator *it) : it(it) {}
  operator import_reference() const;
  bdr_reference &operator=(const import_reference &reference);
  bool operator==(const bdr_reference &other) const {
    return this->it == other.it;
  }
};

class loaded_field_reference_iterator
    : public class_random_access_iterator<
          loaded_field_reference_iterator, bdr_reference<field>,
          sizeof(std::uint32_t) * 2, class_writer> {
  friend class bdr_reference<field>;
  typedef class_random_access_iterator<loaded_field_reference_iterator,
                                       bdr_reference<field>,
                                       sizeof(std::uint32_t) * 2, class_writer>
      super;

 public:
  using super::class_random_access_iterator;
  reference operator*();
  friend void swap(bdr_reference<field> f1, bdr_reference<field> f2);
};

template <>
class bdr_reference<field> : public comparable<bdr_reference<field>> {
 private:
  loaded_field_reference_iterator *it;

 public:
  bdr_reference(loaded_field_reference_iterator *it) : it(it) {}
  operator field() const;
  bdr_reference &operator=(const field &reference);
  bool operator==(const bdr_reference &other) const {
    return this->it == other.it;
  }
  bool operator<(const bdr_reference &other) const {
    field self = *this;
    field nonself = other;
    return self < nonself;
  }
  friend void swap(bdr_reference<field> f1, bdr_reference<field> f2) {
    field field1 = f1, field2 = f2;
    f1 = field2;
    f2 = field1;
  }
};

}  // namespace classloading
}  // namespace oops

#endif