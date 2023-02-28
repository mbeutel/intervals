# intervals: interval-aware programming in C++

| metadata | build  | tests  |
| -------- | ------ | ------ |
| [![Language](https://badgen.net/badge/C++/20+/blue)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) <br> [![License](https://badgen.net/github/license/mbeutel/intervals)](https://opensource.org/licenses/BSL-1.0) <br> [![Version](https://badgen.net/github/release/mbeutel/intervals)](https://github.com/mbeutel/makeshift/releases)   |   [![Build Status](https://dev.azure.com/moritzbeutel/intervals/_apis/build/status/mbeutel.intervals?branchName=master)](https://dev.azure.com/moritzbeutel/intervals/_build/latest?definitionId=2&branchName=master)   |   [![Azure DevOps tests](https://img.shields.io/azure-devops/tests/moritzbeutel/intervals/2)](https://dev.azure.com/moritzbeutel/intervals/_testManagement/runs)  |


*intervals* implements traditional [interval arithmetic](https://en.wikipedia.org/wiki/Interval_arithmetic)
in C++.

Unlike for real numbers, the relational comparison of intervals is ambiguous. Given two intervals
𝑈 = [0, 2] and 𝑉 = [1, 3], what would "𝑈 < 𝑉" mean? Two possible interpretations are often referred to
as "possibly", equivalent to "∃𝑢∈𝑈 ∃𝑣∈𝑉: 𝑢 < 𝑣", and "certainly", equivalent to "∀𝑢∈𝑈 ∀𝑣∈𝑉: 𝑢 < 𝑣".

Both interpretations have their uses. However, we found that just having relational predicates adhere to
one of the given interpretations leads to logical inconsistencies and brittle code. In pursuing our goal
to write *interval-aware code*, we thus define the relational operators (`==`, `!=`, `<`, `>`, `<=`, `>=`)
for intervals as [set-valued operators](#TODO).
Together with a set of [Boolean projections](TODO), these operators ensure
[logical consistency in branch conditions](#TODO) and give rise to a paradigm for
[*interval-aware programming*](#TODO).


## Contents

- [Example usage](#example-usage)
- [License](#license)
- [Compiler and platform support](#compiler-and-platform-support)
- [Dependencies](#dependencies)
- [Installation and use](#installation-and-use)
- [Version semantics](#version-semantics)
- [Motivation](#motivation)
- [Reference documentation](#reference-documentation)
- [Other implementations](#other-implementations)


## Example usage

```c++
#include <intervals/interval.hpp>

template <typename T>
T max3(T a, T b)
{
    using namespace intervals::math;   // for constrain(), assign_partial()
    using namespace intervals::logic;  // for possibly()
    T x = empty;
    auto c = (a < b);
    if (possibly(c))
    {
        auto bc = constrain(b, c);
        assign_partial(x, bc);
    }
    if (possibly(!c))
    {
        auto ac = constrain(a, !c);
        assign_partial(x, ac);
    }
    return x;
}
int main()
{
    auto a = 2.;
    auto b = 3.;
    std::cout << "A = " << a << "\n"
              << "B = " << b << "\n"
              << "max3(a,b) = " << max3(a,b) << "\n\n";  // prints "3"

    auto A = intervals::interval{ 0., 3. };
    auto B = intervals::interval{ 1., 2. };
    std::cout << "A = " << a << "\n"
              << "B = " << b << "\n"
              << "max3(A,B) = " << max3(A,B) << "\n";  // prints "[1, 3]"
}
```


## License

*intervals* uses the [Boost Software License](LICENSE.txt).


## Compiler and platform support

*intervals* is a platform-independent header-only library and should work on all platforms with
a compiler and standard library conforming with the C++20 standard.

The following compilers are officially supported (that is, part of [our CI pipeline](TODO)):

- Microsoft Visual C++ 19.3 (Visual Studio 2022)
- GCC 12 with libstdc++ (tested on Linux and MacOS)
- Clang 14 with libc++ (tested on Windows and Linux)

Please note that Apple Clang is not yet supported because the standard library it ships with does
not implement [C++20 ranges](https://en.cppreference.com/w/cpp/ranges).
 

## Dependencies

* [makeshift](https://github.com/mbeutel/makeshift), a library for lightweight metaprogramming
* [gsl-lite](https://github.com/gsl-lite/gsl-lite), an implementation of the [C++ Core Guidelines Support Library](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-gsl)
* optional (for testing only): [Catch2](https://github.com/catchorg/Catch2)


## Installation and use

### As CMake package

The recommended way to consume *intervals* in your CMake project is to use `find_package()` and `target_link_libraries()`:

```CMake
cmake_minimum_required(VERSION 3.20 FATAL_ERROR)
    
find_package(intervals 1.0 REQUIRED)
    
project(my-program LANGUAGES CXX)
    
add_executable(my-program main.cpp)
target_link_libraries(my-program PRIVATE intervals::intervals)
```

(Right now, installation of the library and its dependencies is still a manual process because
[*makeshift*](https://github.com/mbeutel/makeshift) and *intervals* are not yet available in
the [Vcpkg](https://vcpkg.io/en/index.html) package manager, which we hope to change soon.)

First, clone the repositories of [*gsl-lite*](https://github.com/gsl-lite/gsl-lite) and
[*makeshift*](https://github.com/mbeutel/makeshift) and configure build directories with CMake:

    git clone git@github.com:gsl-lite/gsl-lite.git <gsl-lite-dir>
    cd <gsl-lite-dir>
    mkdir build
    cd build
    cmake -G Ninja ..

    git clone git@github.com:mbeutel/makeshift.git <makeshift-dir>
    cd <makeshift-dir>
    mkdir build
    cd build
    cmake -G Ninja -Dgsl-lite_DIR:PATH=<gsl-lite-dir>/build ..

Then, clone the *intervals* repository and configure a build directory with CMake:

    git clone git@github.com:mbeutel/intervals.git <intervals-dir>
    cd <intervals-dir>
    mkdir build
    cd build
    cmake -G Ninja -Dgsl-lite_DIR:PATH=<gsl-lite-dir>/build -Dmakeshift_DIR:PATH=<makeshift-dir>/build ..

(Note that, as header-only libraries, none of these projects need to be built.)

Now, configure and build your project:

    cd <my-program-dir>
    mkdir build
    cd build
    cmake -G Ninja -Dgsl-lite_DIR:PATH=<gsl-lite-dir>/build -Dmakeshift_DIR:PATH=<makeshift-dir>/build -Dmakeshift_DIR:PATH=<intervals-dir>/build ..
    cmake --build .

### Natvis debugger visualizer

The *intervals* library comes with a [Natvis](https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects?view=vs-2022)
debugger visualizer which can be used by the Visual C++ debugger to inspect intervals, sets,
relational predicates and logical expressions at runtime:

TODO: show demo screenshot

The Natvis file is named "intervals.natvis" and can be found in the "src" subdirectory.
Please refer to the Natvis documentation on
[how to add the visualizer to your project](https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects#add-a-natvis-file-to-a-c-project)
or
[how to make it available to all projects](https://learn.microsoft.com/en-us/visualstudio/debugger/create-custom-views-of-native-objects?view=vs-2022#BKMK_natvis_location).


## Version semantics

*intervals* follows [Semantic Versioning](https://semver.org/) guidelines. We maintain
[API](https://en.wikipedia.org/wiki/Application_programming_interface) and
[ABI](https://en.wikipedia.org/wiki/Application_binary_interface) compatibility and avoid
breaking changes in minor and patch releases.

Development happens in the `master` branch. Versioning semantics apply only to tagged releases:
there is no stability guarantee between individual commits in the `master` branch, that is, anything
added since the last tagged release may be renamed, removed, have the semantics changed, etc. without
further notice.

A minor-version release will be compatible (in both ABI and API) with the previous minor-version
release. Thus, once a change is released, it becomes part of the API.


## Motivation

*Note: this section only briefly recounts some basics of interval arithmetic (TODO: does it summarize
the raison d'être of this library?). For the full story, please refer to our accompanying paper
[\[1\]](#references).*

Let a unary function 𝑓: 𝒮 → ℝ be defined for a set 𝒮 ⊆ ℝ. For some subset 𝒰 ⊆ 𝒮, the *set extension*
of 𝑓 is then defined as the set-valued function  
  
𝑓(𝒰) := { 𝑓(𝑥) | 𝑥 ∈ 𝒰 } .
  
The *interval enclosure* ℐ\[𝒮\] of a non-empty set 𝒮 ⊆ ℝ is defined as the minimal enclosing interval,

ℐ\[𝒮\] := [inf 𝒮, sup 𝒮] .

Let the set of all closed intervals in a set 𝒮 ⊆ ℝ be denoted as \[𝒮\]. An interval-valued function
𝐹: [𝒮] → [ℝ] is called an *interval extension* of 𝑓 if it satisfies the *inclusion property*:

∀𝑋∈[𝒮] ∀𝑥∈𝑋: 𝑓(𝑥) ∈ 𝐹(𝑋) .

Analogous definitions can be made for functions of higher arity.

Most mathematical functions defined by *intervals* implement the *precise interval extension* of the
eponymous functions defined for the underlying arithmetic type. The precise interval extension
is identical to the *interval enclosure* of the *set extension*. 

TODO:
- fundamental theorem
- interpretations of relational operators
- set-valued relational predicates
- constraining


## Reference documentation

*intervals* mainly consists of the two class templates [`interval<>`](#interval) and [`set<>`](#set)
along with supporting infrastructure.

**Overview:**
- [`interval<>`](#interval)
- [`set<>`](#set)
- [Relational operators and constraints](#relational-operators-and-constraints)
- [Auxiliary types and aliases](#auxiliary-types-and-aliases)

### `interval<>`

```c++
template <typename T>
class interval;
```

`interval<T>` represents a closed interval whose endpoints are of type `T`. The following type arguments
are supported:
- floating-point types: one of `float`, `double`, `long double`;
- signed integral types such as `int`;
- random-access iterators such as `std::string::iterator`.

**Overview:**
- [Construction](#construction)
- [Accessors](#accessors)
- [Assignment](#assignment)
- [Mathematical functions](#mathematical-functions)
- [Stream formatting](#stream-formatting)
- [Conversion](#conversion)

#### Construction

An instance of an object of type `interval<T>` can be created through one of its constructors:
- The default constructor, which creates an object with an invalid state:
  ```c++
  auto U = interval<int>{ };
  assert(!U.assigned());
  ```
  The member function `U.assigned()` can be used to determine whether the interval has been
  assigned a value.  
  Note that the default constructor is not available for the specialization where `T` is a
  random-access iterator; this specialization has no invalid state.
- The converting constructor, which accepts an argument of the endpoint type `T`:
  ```c++
  auto V = interval{ 2. };  // represents degenerate interval [2, 2]
  assert(V.matches(2.));
  ```
- The endpoint constructor, which accepts the two interval endpoints a and b of type `T`,
  where a ≤ b must hold:
  ```c++
  auto W = interval{ 0., 1. };  // represents interval [0, 1]
  //auto Z = interval{ 1., 0. };  // this would fail with a GSL precondition violation
  ```

#### Accessors

For an object `X` of type `interval<T>`, the interval bounds can be determined as `X.lower()` and
`X.upper()`:
```c++
//U.lower();  // this would fail with a GSL precondition violation
assert(V.lower() == V.upper());
assert(W.lower() == 0.);
assert(W.upper() == 1.);
```
Alternatively, an interval can be decomposed into its bounds using [structured bindings](https://en.cppreference.com/w/cpp/language/structured_binding):
```c++
auto [a, b] = W;
assert(a == 0.);
assert(b == 1.);
```
To access the interval bounds without the runtime validity check, use `X.lower_unchecked()`
and `X.upper_unchecked()`.

The single value of a degenerate interval `X = interval{ x, x }` can be accessed with the member function
`X.value()`:
```c++
//U.value();  // this would fail with a GSL precondition violation
assert(V.value() == 2.);
//W.value();  // this would fail with a GSL precondition violation
```

To check whether an interval `Y = interval{ a, b }` *contains* another interval `Z = interval{ c, d }`
of the same type – that is, whether 𝑐 ≥ 𝑎 ∧ 𝑑 ≤ 𝑏 –, the member function `Y.contains()` can be used:
```c++
assert(interval{ 0., 3. }.contains(1.));
assert(interval{ 0., 3. }.contains(interval{ 0., 1. }));
assert(!interval{ 0., 3. }.contains(interval{ 1., 4. }));
```
An interval `Y = interval{ a, b }` is said to *enclose* another interval `Z = interval{ c, d }` if
𝑐 > 𝑎 ∧ 𝑑 < 𝑏, which can be checked with the member function `Y.encloses()`:
```c++
assert(interval{ 0., 3. }.encloses(1.));
assert(!interval{ 0., 3. }.encloses(0.));
assert(interval{ 0., 3. }.encloses(interval{ 1., 2. }));
assert(!interval{ 0., 2. }.encloses(interval{ 0., 1. }));
assert(!interval{ 0., 2. }.encloses(interval{ -1., 1. }));
```
An interval `Y = interval{ a, b }` *matches* another interval `Z = interval{ c, d }` if the endpoints
of the intervals match exactly, 𝑐 = 𝑎 ∧ 𝑑 = 𝑏, which can be checked with the member function `Y.matches()`:
```c++
assert(!interval{ 0., 3. }.matches(0.));
assert(interval{ 0., 3. }.matches(interval{ 0., 3. }));
assert(!interval{ 0., 2. }.matches(interval{ 0., 1. }));
```

#### Assignment

TODO: `reset()`, `assign()`, `assign_partial()`

#### Mathematical functions

A variety of mathematical functions and operators is defined for arguments of type `interval<T>`.
For binary functions and operators, at least one operand must be of type `interval<T>`.
The other operand may either be of type `interval<U>` or of type `U`, where `T` and `U` have a
[common type](https://en.cppreference.com/w/cpp/types/common_type), and the function returns
an object of type `interval<std::common_type_t<T, U>>`.
For example:
```c++
auto U = interval{ 2 } + interval{ 0., 1. };  // `interval<int>` + `interval<double>` → `interval<double>`
auto V = interval{ 3.f } + 2.;  // `interval<float>` + `double` → `interval<double>`
```

The following mathematical functions are defined for `interval<T>` for a floating-point type argument `T`
as the [interval extension](#motivation) (precise unless indicated otherwise) of the respective real-valued
function:
- The unary arithmetic operators `-` and `+` and the binary arithmetic operators `+`, `-`, `*`, and `/`.
- `min(U, V)` and `max(U, V)`.
- `square(U)` and `cube(U)`, corresponding to the functions  
  𝑠𝑞𝑢𝑎𝑟𝑒: ℝ → ℝ, 𝑥 ↦ 𝑥²  
  and  
  𝑐𝑢𝑏𝑒: ℝ → ℝ, 𝑥 ↦ 𝑥³.
- [`sqrt(U)`](https://en.cppreference.com/w/cpp/numeric/math/sqrt) and
  [`cbrt(U)`](https://en.cppreference.com/w/cpp/numeric/math/cbrt), corresponding to the √ and ∛ functions.
- [`abs(U)`](https://en.cppreference.com/w/cpp/numeric/math/abs).
- [`log(U)`](https://en.cppreference.com/w/cpp/numeric/math/log),
  [`exp(U)`](https://en.cppreference.com/w/cpp/numeric/math/exp), and
  [`pow(U, V)`](https://en.cppreference.com/w/cpp/numeric/math/pow).
- The trigonometric functions [`sin(U)`](https://en.cppreference.com/w/cpp/numeric/math/sin),
  [`cos(U)`](https://en.cppreference.com/w/cpp/numeric/math/cos), and
  [`tan(U)`](https://en.cppreference.com/w/cpp/numeric/math/tan) and their inverses
  [`asin(U)`](https://en.cppreference.com/w/cpp/numeric/math/asin),
  [`acos(U)`](https://en.cppreference.com/w/cpp/numeric/math/acos), and
  [`atan(U)`](https://en.cppreference.com/w/cpp/numeric/math/atan).
- [`atan2(V, U)`](https://en.cppreference.com/w/cpp/numeric/math/atan2).
- [`floor(U)`](https://en.cppreference.com/w/cpp/numeric/math/floor),
  [`ceil(U)`](https://en.cppreference.com/w/cpp/numeric/math/ceil), and
  [`round(U)`](https://en.cppreference.com/w/cpp/numeric/math/round).
- `frac(U)`, corresponding to the function 𝑓𝑟𝑎𝑐: ℝ → ℝ, x ↦ x - floor(x).
- `fractional_weights(U)`, corresponding to the function  
  𝑓𝑟𝑎𝑐𝑡𝑖𝑜𝑛𝑎𝑙_𝑤𝑒𝑖𝑔ℎ𝑡𝑠: ℝ² → ℝ², (𝑎,𝑏) ↦ (𝑎/(𝑎 + 𝑏), 𝑏/(𝑎 + 𝑏)).
- `blend_linear(U)`, as the non-precise interval extension of the function  
  𝑏𝑙𝑒𝑛𝑑_𝑙𝑖𝑛𝑒𝑎𝑟: ℝ² × ℝ² → ℝ, ((𝑎,𝑏), (𝑥,𝑦)) ↦ 𝑎/(𝑎 + 𝑏)⋅x + 𝑏/(𝑎 + 𝑏)⋅y.
- `blend_quadratic(U)`, as the non-precise interval extension of the function  
  𝑏𝑙𝑒𝑛𝑑_𝑞𝑢𝑎𝑑𝑟𝑎𝑡𝑖𝑐: ℝ² × ℝ² → ℝ, ((𝑎,𝑏), (𝑥,𝑦)) ↦ √\[(𝑎/(𝑎 + 𝑏)⋅𝑥)² + (𝑏/(𝑎 + 𝑏)⋅𝑦)²\].

The following mathematical functions are defined for `interval<T>` for a signed integral type argument `T`
as the precise interval extension of the respective integer-valued function:
- The unary arithmetic operators `-` and `+` and the binary arithmetic operators `+`, `-`, `*`, and `/`.
- `min(U, V)` and `max(U, V)`.
- `square(U)` and `cube(U)`, corresponding to the functions  
  𝑠𝑞𝑢𝑎𝑟𝑒: ℤ → ℤ, 𝑥 ↦ 𝑥²  
  and  
  𝑐𝑢𝑏𝑒: ℤ → ℤ, 𝑥 ↦ 𝑥³.
- [`abs(U)`](https://en.cppreference.com/w/cpp/numeric/math/abs).

The following mathematical functions are defined for `interval<T>` for a random-access iterator type
argument `T` as the precise interval extension of the respective iterator-valued function:
- `operator +(U, V)` (offset iterator) where one argument is convertible to type `interval<T>`, and the
  other is an integer or an interval of integers.
- `operator -(U, V)` (offset iterator) where `U` is convertible to type `interval<T>`, and `V` is an
  integer or an interval of integers.
- `operator -(U, V)` (iterator difference) where both arguments are convertible to type `interval<T>`.
- `prev(U)` and `next(U)`.

#### Stream formatting

Intervals can be written to a stream using the `<<` operator. For degenerate intervals holding
only a single value, only the single endpoint is written.
```c++
//std::cout << U;  // this would fail with a GSL precondition violation
std::cout << "V = " << V << "\n";  // prints "V = 2"
std::cout << "W = " << W << "\n";  // prints "W = [0, 1]"
```

#### Conversion

Type conversions between different instantiations of `interval<>` are possible if the type arguments
are convertible. `interval<>` has an implicit conversion constructor for this purpose:
```c++
auto U = interval{ 0, 1 };  // `interval<int>`
interval<double> V = U;  // implicit conversion
```

Intervals can also be converted explicitly using one of the following casts inspired by the
[Guidelines Support Library](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#gslutil-utilities):
- `narrow_cast<T>(U)`, which creates an object of type `interval<T>` from the interval `U` by casting
  the lower and upper bounds to `T` with a `static_cast<>()`.
  ```c++
auto U = interval{ 0., 0.5 };  // `interval<double>`
auto V = narrow_cast<int>(U);  // explicit lossy conversion to `interval<int>{ 0 }`
```
- `narrow_failfast<T>(U)`, which behaves like `narrow_cast<T>(U)` but uses a GSL precondition check to
  assert that the value of `U` can be represented exactly by type `interval<T>`.
  ```c++
auto U = interval{ 0., 0.5 };  // `interval<double>`
auto V = narrow_failfast<float>(U);  // explicit conversion to `interval<float>{ 0.f, 0.5f }`
//auto V = narrow_failfast<int>(U);  // this would fail with a GSL precondition violation
```
- `narrow<T>(U)`, which behaves like `narrow_cast<T>(U)` but throws a `gsl_lite::narrowing_error` exception
  if the value of `U` cannot be represented exactly by type `interval<T>`.
  ```c++
auto U = interval{ 0., 0.5 };  // `interval<double>`
auto V = narrow<float>(U);  // explicit conversion to `interval<float>{ 0.f, 0.5f }`
//auto V = narrow<int>(U);  // this would raise a `gsl_lite::narrowing_error` exception
```


### `set<>`

TODO:
- `values`
- default constructor
- value constructor
- `initializer_list<>` constructor
- `from_bits()` and `to_bits()`
- `contains()`, `contains_index()`, `matches()`, `value()`
- stream formatting
- logical operators `!`, `&`, `|`, `^`

### Relational operators and constraints

TODO:
- relational comparison operators for `interval<>` and `set<>`
- `possibly()`, `possibly_not()`
- `always()`, `never()`
- `contingent()`, `vacuous()`
- `constrain()`

### Algorithms

TODO:
- `at()`
- `enumerate()`
- `lower_bound()`
- `upper_bound()`
- `partition_point()`

### Utilities

TODO:
- `sign`
- `if_else()`
- `as_regular<>`



## Other implementations

Many other libraries for interval arithmetics exist. Some exemplary C++ libraries are
[Boost Interval Arithmetic Library](https://www.boost.org/doc/libs/1_81_0/libs/numeric/interval/doc/interval.htm)
and [GAOL](https://frederic.goualard.net/#research-software-gaol).

*intervals* differs from other libraries mostly by its handling of relational comparison operators.


## References

The *intervals* library has been motivated and introduced in the following CoNGA 2023 conference paper:

Beutel & Strzodka, *A paradigm for interval-aware programming*, in press
