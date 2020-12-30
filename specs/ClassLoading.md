# Class Loading Process

## Outline

## Introduction

Class loading is a complicated process, inherently recursive and resource-intensive. Key constraints on the design of how classes are loaded include 1. ensuring that garbage collection does not occur during the class loading process, as updating pointers inside native code is difficult and error-prone, and 2. ensuring that the class file is valid, all while trying to not run into other resource constraints such as out-of-memory or excessive computation. Such a large block of code also invites bugs, so writing code that is self-explanatory while also exploiting compiler type-safety guarantees to ensure correctness is critical to minimizing debugging effort.
