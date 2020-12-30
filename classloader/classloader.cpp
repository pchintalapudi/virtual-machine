#include "classloader.h"

using namespace oops::classloading;
std::optional<oops::classes::clazz> classloader::load_class(classes::string str) {
    return this->load_class_unwrapped(str.to_char_array(), str.length());
}

std::optional<oops::classes::clazz> classloader::load_class_unwrapped(
    const char *cstr, std::int32_t length) {
  auto str = std::string(cstr, length);
  if (this->loading_classes.find(str) != this->loading_classes.end()) {
    return {};
  }
  if (auto cls = this->cached_classes.find(str);
      cls != this->cached_classes.end()) {
    if (cls->second.get_raw() == nullptr) {
      return {};
    } else {
      return cls->second;
    }
  }
  this->loading_classes.insert(str);
  auto loaded = this->impl_load_class(cstr, str.length());
  this->loading_classes.erase(str);
  if (!loaded) {
    this->cached_classes.emplace(str, classes::clazz(nullptr));
  } else {
    this->cached_classes.emplace(str, *loaded);
  }
  return loaded;
}

bool classloader::is_superclass(classes::clazz maybe_super,
                                classes::clazz maybe_sub) {
  return this->instanceof_table.is_superclass(maybe_super, maybe_sub);
}