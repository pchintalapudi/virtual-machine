# Field Access Protocol

## Class Field Retrieval

---

1. Load the indexed field descriptor from the currently executing class context
2. Load the class descriptor from the field descriptor from the currently executing class context
3. Load the class from which to retrieve the field
4. Reflect the field index from the loaded class
5. Load the value from the loaded class with the associated field index

Steps 2-3 may optionally be cached, as can steps 1 and 4

## Class Field Setting

---

1. Load the indexed field descriptor from the currently executing class context
2. Load the class descriptor from the field descriptor from the currently executing class context
3. Load the class from which to retrieve the field
4. Reflect the field index from the loaded class
5. Set the value from the loaded class with the associated field index

Steps 2-3 may optionally be cached, as can steps 1 and 4

## Object Field Retrieval

---

1. Load the indexed field descriptor from the currently executing class context
2. Load the class descriptor from the field descriptor from the currently executing class context
3. Load the class from which to retrieve the field
4. Reflect the field index from the loaded class
5. Load the value from the object with the associated field index

Steps 2-3 may optionally be cached, as can steps 1 and 4

## Object Field Setting

---

1. Load the indexed field descriptor from the currently executing class context
2. Load the class descriptor from the field descriptor from the currently executing class context
3. Load the class from which to retrieve the field
4. Reflect the field index from the loaded class
5. Set the value from the object with the associated field index

Steps 2-3 may optionally be cached, as can steps 1 and 4
