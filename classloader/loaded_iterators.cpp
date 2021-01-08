#include "loaded_iterators.h"

using namespace oops::classloading;

loaded_class_reference_iterator::reference loaded_class_reference_iterator::operator*() {
    return reference(this);
}

loaded_import_reference_iterator::reference loaded_import_reference_iterator::operator*() {
    return reference(this);
}

loaded_field_reference_iterator::reference loaded_field_reference_iterator::operator*() {
    return reference(this);
}