# Calling Convention

## Method Lookup - Caller

### Static Methods

1. Load static method import from current class
1. Load associated class import from current class
1. Load associated class into VM
1. Lookup method offset in associated class
1. Get method directly from associated class

### Virtual Methods

1. Load virtual method import from current class
1. Load associated class import from current class
1. Load associated class into VM
1. Lookup virtual method offset in associated class
1. Lookup method in object's dynamic class' virtual method table

### Dynamic Methods

1. Lookup dynamic method import from current class
1. Get object's dynamic class
1. Lookup method offset in dynamic class
1. Get method directly from associated class

## Method Prologue - Caller

1. Check argument types being passed in
1. Write arguments in order into appropriate sections of the stack
1. Write return address into stack
1. Write return stack offset into stack
1. Write stack frame pointer into stack
1. Update executing method

## Method Prologue - Callee

1. Set next instruction pointer to 0
1. Proceed to execution

## Method Epilogue - Callee

1. Write return value in return stack offset
1. Restore previous method

## Method Epilogue - Caller

1. Restore instruction pointer
1. Pop stack frame
