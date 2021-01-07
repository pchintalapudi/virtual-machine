#ifndef OOPS_CLASSLOADER_LOADED_ITERATORS_H
#define OOPS_CLASSLOADER_LOADED_ITERATORS_H

#include "class_file_io.h"
#include "loader_iterators.h"

namespace oops {
namespace classloading {

template <typename payload>
class bdr_reference {};

template <>
class bdr_reference<class_reference>
    : public comparable<bdr_reference<class_reference>> {
 private:
  void *ptr;  // TODO FIXME
 public:
  operator class_reference() const;
  bdr_reference &operator=(const class_reference &reference);
  bool operator==(const bdr_reference &other) const;
  bool operator<(const bdr_reference &other) const;
};


class loaded_class_reference_iterator
    : public class_random_access_iterator<loaded_class_reference_iterator,
                                          bdr_reference<class_reference>,
                                          sizeof(std::uint32_t), class_writer> {
 public:
  reference operator*();
};

template <>
class bdr_reference<import_reference>
    : public comparable<bdr_reference<import_reference>> {
 private:
  void *ptr;  // TODO FIXME
 public:
  operator import_reference() const;
  bdr_reference &operator=(const import_reference &reference);
  bool operator==(const bdr_reference &other) const;
  bool operator<(const bdr_reference &other) const;
};


class loaded_import_reference_iterator
    : public class_random_access_iterator<loaded_import_reference_iterator,
                                          bdr_reference<import_reference>,
                                          sizeof(std::uint32_t), class_writer> {
 public:
  reference operator*();
};

template <>
class bdr_reference<field> : public comparable<bdr_reference<field>> {
 private:
  void *ptr;  // TODO FIXME
 public:
  operator field() const;
  bdr_reference &operator=(const field &reference);
  bool operator==(const bdr_reference &other) const;
  bool operator<(const bdr_reference &other) const;
  friend void swap(bdr_reference<field> f1, bdr_reference<field> f2);
};

class loaded_field_reference_iterator
    : public class_random_access_iterator<
          loaded_field_reference_iterator, bdr_reference<field>,
          sizeof(std::uint32_t) * 2, class_writer> {
 public:
  reference operator*();
};

}  // namespace classloading
}  // namespace oops

#endif