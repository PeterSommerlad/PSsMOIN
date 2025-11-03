# PSsMOIN
Simple Safer Integer Replacement types with Modular (Wrapping) Arithmetic for C++20

![tests](https://github.com/PeterSommerlad/PSsMOIN/actions/workflows/runtests.yml/badge.svg?branch=main)

Note: This is a rename and extension of my earlier attempt in [PSsimplesafeint](https://github.com/PeterSommerlad/PSsimplesafeint), if you need wrap around integer replacement types for C++17 look there for the C++17 branch.

This is a new version of this funcitonality (simpe modulo arithmetic integers) with 2 C++20 only implementations, one same as this one with `enum class` types and another one with class types wrapping the integers. While as of today both gcc and clang generate (almost) identical code for both versions, other compilers create much less efficient code for the class types than for the enumeration types. Note, types should only be a compile-time thing (unless they aren't).

The error handling is still somehow configurable but with fewer knobs. You either get an exception throwing a `char const *` with an error message or one can opt for getting a signal `SIGFPE` for invalid operations.



It completes the three approaches for adding safer integer arithmetic:

* overflow detection [PSsODIN](https://github.com/PeterSommerlad/PSsODIN)
* saturation arithmetic [PSsSATIN](https://github.com/PeterSommerlad/PSsSATIN)
