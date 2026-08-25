# PSsMOIN
Simple Safer Integer Replacement types with Modular (Wrapping) Arithmetic for C++20

![tests](https://github.com/PeterSommerlad/PSsMOIN/actions/workflows/runtests.yml/badge.svg?branch=main)

Note: This is a rename and extension of my earlier attempt in [PSsimplesafeint](https://github.com/PeterSommerlad/PSsimplesafeint).

If you need wrap around integer replacement types for C++17, e.g., because you need to be MISRA-C++:2023 compliant,  look for the C++17 branch "std17".

This is a new version of this funcitonality (simpe modulo arithmetic integers) with 2 C++20 only implementations, one same as this one with `enum class` types [pssmoin.h](https://github.com/PeterSommerlad/PSsMOIN/blob/main/include/pssmoin.h) and another one with class types wrapping the integers [moins.h](https://github.com/PeterSommerlad/PSsMOIN/blob/main/include/moins.h). While as of today both gcc and clang generate (almost) identical code for both versions, other compilers create much less efficient code for the class types than for the enumeration types. Note, IMHO types should only be a compile-time thing (unless they aren't, e.g., `virtual` is used).

The error handling is still somehow configurable but with fewer knobs. You either get an exception throwing a `char const *` with an error message or one can opt for getting a signal `SIGFPE` for invalid operations in addition.

To support the use with units libraries, such as mp_units, the current version supports multiplication with and division by other integral types, as long as the integral operand is in range of the underlying type.

Play with it on [compiler explorer](https://godbolt.org/z/K3WPPrcoE).

It completes the three approaches for adding safer integer arithmetic:

* overflow detection [PSsODIN](https://github.com/PeterSommerlad/PSsODIN)
* saturation arithmetic [PSsSATIN](https://github.com/PeterSommerlad/PSsSATIN)
