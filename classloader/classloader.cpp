#include "classloader.h"

#include "classloader_impl.h"

using namespace oops::classloading;

std::optional<oops::classes::clazz> classloader::load_class(
    oops::classes::string str) {
  char *cstr = str.to_char_array();
  if (auto cls = this->cached_classes.find(cstr);
      cls != this->cached_classes.end()) {
    if (cls->second.get_raw() == nullptr) {
      return {};
    } else {
      return cls->second;
    }
  }
  auto loaded = impl_load_class(cstr, str.length());
  if (!loaded) {
    this->cached_classes.emplace(cstr, classes::clazz(nullptr));
  } else {
    this->cached_classes.emplace(cstr, *loaded);
  }
  return loaded;
}

bool classloader::is_superclass(classes::clazz maybe_super, classes::clazz maybe_sub) {
    return this->instanceof_table.is_superclass(maybe_super, maybe_sub);
}