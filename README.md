# intervals: interval-aware programming in C++

| metadata | build  | tests  |
| -------- | ------ | ------ |
| [![Language](https://badgen.net/badge/language/C++%E2%89%A520/blue)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization) <br> [![License](https://badgen.net/github/license/mbeutel/intervals)](https://opensource.org/licenses/BSL-1.0) <br> [![Version](https://badgen.net/github/release/mbeutel/intervals)](https://github.com/mbeutel/intervals/releases)   |   [![Build Status](https://dev.azure.com/moritzbeutel/intervals/_apis/build/status/mbeutel.intervals?branchName=master)](https://dev.azure.com/moritzbeutel/intervals/_build/latest?definitionId=9&branchName=master)   |   [![Azure DevOps tests](https://img.shields.io/azure-devops/tests/moritzbeutel/intervals/9)](https://dev.azure.com/moritzbeutel/intervals/_testManagement/runs)  |


*intervals* implements traditional [interval arithmetic](https://en.wikipedia.org/wiki/Interval_arithmetic)
in C++.

Unlike for real numbers, the relational comparison of intervals is ambiguous. Given two intervals
𝑈 = [0,2] and 𝑉 = [1,3], what would "𝑈 < 𝑉" mean? Two possible interpretations are often referred to
as "possibly" (∃𝑢∈𝑈 ∃𝑣∈𝑉: 𝑢 < 𝑣) and "certainly" (∀𝑢∈𝑈 ∀𝑣∈𝑉: 𝑢 < 𝑣).

Both interpretations have their uses. However, we found that just defining relational predicates in
accordance with one of the given interpretations leads to logical inconsistencies and brittle code.
In pursuing our goal to write *interval-aware code*, we thus define the relational operators
(`==`, `!=`, `<`, `>`, `<=`, `>=`) for intervals as [set-valued operators](#set-valued-logic).
Together with a set of [Boolean projections](#boolean-projections), these operators ensure
[logical consistency](#relational-predicates) in branch conditions and give rise to a paradigm for
[*interval-aware programming*](#interval-aware-programming).


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
#include <iostream>

#include <intervals/interval.hpp>

template <typename T>
T max3(T a, T b)
{
    using namespace intervals::math;   // for constrain(), assign_partial()
    using namespace intervals::logic;  // for possibly()
    auto x = T{ };
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
              << "max3(a,b) = " << max3(a,b) << "\n\n";  // prints "max3(a,b) = 3"

    auto A = intervals::interval{ 0., 3. };
    auto B = intervals::interval{ 1., 2. };
    std::cout << "A = " << a << "\n"
              << "B = " << b << "\n"
              << "max3(A,B) = " << max3(A,B) << "\n";  // prints "max3(A,B) = [1, 3]"
}
```


## License

*intervals* uses the [Boost Software License](LICENSE.txt).


## Compiler and platform support

*intervals* is a platform-independent header-only library and should work on all platforms with
a compiler and standard library conforming with the C++20 standard.

The following compilers are officially supported (that is, part of
[our CI pipeline](https://dev.azure.com/moritzbeutel/intervals/_build/latest?definitionId=9&branchName=master)):

- Microsoft Visual C++ 19.3 and newer (Visual Studio 2022 and newer)
- GCC 12 and newer with libstdc++ (tested on Linux and MacOS)
- Clang 14 and newer with libc++ (tested on Windows and Linux)
- AppleClang 14.0.3 and newer with libc++ (Xcode 14.3 and newer)


## Dependencies

* [*gsl-lite*](https://github.com/gsl-lite/gsl-lite), an implementation of the [C++ Core Guidelines Support Library](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-gsl)
* [*makeshift*](https://github.com/mbeutel/makeshift), a library for lightweight metaprogramming
* optional (for testing only): [*Catch2*](https://github.com/catchorg/Catch2)


## Installation and use

The recommended way to consume *intervals* in your CMake project is to use `find_package()` to locate the package `intervals`
and `target_link_libraries()` to link to the imported target `intervals::intervals`:

```CMake
cmake_minimum_required(VERSION 3.30 FATAL_ERROR)

project(my-program LANGUAGES CXX)

find_package(intervals 1.0 REQUIRED)

add_executable(my-program main.cpp)
target_compile_features(my-program PRIVATE cxx_std_20)
target_link_libraries(my-program PRIVATE intervals::intervals)
```

*intervals* may be obtained with [CPM](https://github.com/cpm-cmake/CPM.cmake):
```cmake
# obtain dependencies
CPMAddPackage(NAME gsl-lite  VERSION 0.43.0 GITHUB_REPOSITORY gsl-lite/gsl-lite)
CPMAddPackage(NAME makeshift GIT_TAG master GITHUB_REPOSITORY mbeutel/makeshift)

# obtain intervals
CPMAddPackage(NAME intervals VERSION 1.0.0 GITHUB_REPOSITORY mbeutel/intervals)
```
See the directories [example/with-CPM](https://github.com/mbeutel/intervals/tree/master/example/with-CPM) for
an example project that uses CPM to obtain *intervals*.


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

*intervals* follows [Semantic Versioning](https://semver.org/) guidelines with regard to its
[API](https://en.wikipedia.org/wiki/Application_programming_interface). We aim to retain full
API compatibility and to avoid breaking changes in minor and patch releases.

We do not guarantee to maintain [ABI](https://en.wikipedia.org/wiki/Application_binary_interface)
compatibility except in patch releases.

Development happens in the `master` branch. Versioning semantics apply only to tagged releases:
there is no stability guarantee between individual commits in the `master` branch, that is, anything
added since the last tagged release may be renamed, removed, have the semantics changed, etc. without
further notice.

A minor-version release will be API-compatible with the previous minor-version release. Thus, once
a change is released, it becomes part of the API.


## Motivation

This section  gives a brief recap of some basics of interval arithmetic and summarizes
the raison d'être of this library. For the full story, please refer to our accompanying paper
[\[1\]](#references).

**Overview:**
- [Set and interval arithmetic](#set-and-interval-arithmetic)
- [Fundamental theorem of interval arithmetic](#fundamental-theorem-of-interval-arithmetic)
- [Dependency problem](#dependency-problem)
- [Algorithms](#algorithms)
- [Utilities](#utilities)


### Set and interval arithmetic

Let a unary function 𝑓: 𝒮 → ℝ be defined for a set 𝒮 ⊆ ℝ. For some subset 𝒰 ⊆ 𝒮, the *set extension*
of 𝑓 is then defined as the set-valued function

𝑓(𝒰) := { 𝑓(𝑥) | 𝑥 ∈ 𝒰 } .

The *interval enclosure* ℐ\[𝒮\] of a non-empty set 𝒮 ⊆ ℝ is defined as the minimal enclosing interval,

ℐ\[𝒮\] := [inf 𝒮, sup 𝒮] .

Let the set of all closed intervals in a set 𝒮 ⊆ ℝ be denoted as \[𝒮\]. An interval-valued function
𝐹: [𝒮] → [ℝ] is called an *interval extension* of 𝑓 if it satisfies the *inclusion property*:

∀𝑋 ∈ [𝒮] ∀𝑥 ∈ 𝑋: 𝑓(𝑥) ∈ 𝐹(𝑋) .

Analogous definitions can be made for functions of higher arity.

Interval extensions can be defined for all basic arithmetic operations such as +, -, ⋅, √.
Some examples:

- [𝐴⁻,𝐴⁺] + [B⁻,B⁺] := [𝐴⁻ + 𝐵⁻, 𝐴⁺ + 𝐵⁺]
- -[𝐴⁻,𝐴⁺] := [-𝐴⁺,-𝐴⁻]
- √𝐴 := [√𝐴⁻,√𝐴⁺]

An interval extension 𝐹: [𝒮] → [ℝ] of some function 𝑓: 𝒮 → ℝ is called *precise* if it is identical to
the interval enclosure of the set extension ℐ\[𝐹(𝒮)\] for every argument 𝑋∈[𝒮].

Most mathematical functions defined by the *intervals* library implement the precise interval extension
of the eponymous functions defined for the underlying arithmetic type.

### Fundamental theorem of interval arithmetic

Let a function

𝑓: 𝒮 → ℝ,  𝑥 ↦ 𝑓(𝑥)

be composed of interval-extensible operations, for example 𝑓(𝑥) = 𝑥² − 2𝑥.

Then construct a function

𝐹: [𝒮] → [ℝ], 𝑋 ↦ 𝐹(𝑋)

by replacing operations with their interval extensions. For the given example, this would be
𝐹(𝑋) = 𝑋² − 2𝑋. The "Fundamental Theorem of Interval Arithmetic" [\[2,3\]](#references) then
states that *𝐹 is an interval extension of 𝑓*.

Observing that the interval-valued function 𝐹 is syntactically identical to the real-valued function
𝑓, we assume that it may be possible to repurpose numerical code for interval arithmetic.
This theorem forms the basis of *interval-aware programming*.

### Dependency problem

Although many numerical calculations can be retrofitted for intervals, yielding correct results
as per the fundamental theorem of interval arithmetic, the results are often suboptimal, which means
that the resulting intervals are too wide. The most extreme example would be a function 𝑓(𝑥) = 𝑥 − 𝑥.
If we construct its interval extension as 𝐹(𝑋) = 𝑋 − 𝑋, we find that it yields much wider intervals
than necessary:

𝐹([0,1]) = [0,1] − [0,1] = [0,1] + [−1,0] = [−1,1] ,

whereas the interval extension 𝐺(𝑋) = [0,0] of the real-valued function 𝑔(𝑥) = 0, obviously
algebraically equivalent to 𝑓, always yields the optimally tight interval [0,0].

This is known as the [*dependency problem*](https://en.wikipedia.org/wiki/Interval_arithmetic#Dependency_problem),
and it is one of the reasons why making effective use of interval arithmetic is hard. When making an
existing numerical routine interval-aware, you should expect to spend a substantial amount of work
on rewriting your expressions in algebraically advantageous forms.

### Relational predicates

We note that the relational predicates =, ≠, <, >, ≤, ≥ are ambiguous for interval arguments because
of the possibility of overlapping intervals. For example, given two intervals 𝑈 = [0,2] and 𝑉 = [1,3],
what could be the meaning of "𝑈 < 𝑉"?

Two interpretations are often used in conjunction with interval arithmetic:

- "possibly" semantics, where 𝑈 < 𝑉 :⇔ (∃𝑢 ∈ 𝑈 ∃𝑣 ∈ 𝑉: 𝑢 < 𝑣);
- "certainly" semantics, where 𝑈 < 𝑉 :⇔ (∀𝑢 ∈ 𝑈 ∀𝑣 ∈ 𝑉: 𝑢 < 𝑣).

With either interpretation, however, we find some essential relational identities invalidated:

- 𝐴 ≠ 𝐵    ⇔    ¬(𝐴 = 𝐵)
- 𝐴 < 𝐵    ⇔    ¬(𝐴 ≥ 𝐵)      (complementarity)
- A = 𝐵    ⇔    ¬(𝐴 < 𝐵 ∨ 𝐴 > 𝐵)    (totality)

With regard to relational predicates, we say that a system of relations is *logically consistent*
if these identities hold. But as we find easily by evaluating both sides with the counterexample
A = 𝐵 = [0,1], neither of the given expressions are equivalent for intervals with either "possibly"
or "certainly" semantics. This is unfortunate because these identities are commonly taken for
granted by programmers. For example, consider a straightforward (though perhaps verbose)
implementation of the `max` function:

```c++
template <typename T>
T max(T a, T b)
{
    T x;
    if (a < b)
    {
        x = b;
    }
    else
    {
        x = a;
    }
    return x;
}
```

Semantically, the `else` clause is shorthand for an `if` clause with the negation of the previous
branch condition, `!(a < b)`. However, the *intended* branch condition for the second clause
is actually `a >= b`!

We can see that, with this tentative definition, the interval instantiation of the `max()`
function template does not constitute an interval extension of the instantiation of `max()` for
scalar arguments. For example, consider the three intervals [2,4], [4,5], and [3,6].
According to the [set extension](#set-and-interval-arithmetic), the maximum of two intervals is

max(𝐴,𝐵) := { max(𝑎,𝑏) | a ∈ 𝐴, 𝑏 ∈ 𝐵 } ,

and thus max([2,4],[3,6]) = [3,6] and max([4,5],[3,6]) = [4,6]. However, we find that, if the
relational predicates follow either "possibly" or "certainly" semantics, the results returned
by `max()` are incorrect:

```c++
auto A = interval{ 2, 4 };
auto B = interval{ 4, 5 };
auto C = interval{ 3, 6 };

auto maxAC = max(A, C);
// with "possibly" semantics:  interval{ 3, 6 }  (optimal)
// with "certainly" semantics:  interval{ 2, 4 }  (wrong)

auto maxCA = max(C, A);
// with "possibly" semantics:  interval{ 2, 4 }  (wrong)
// with "certainly" semantics:  interval{ 3, 6 }  (optimal)

auto maxBC = max(B, C);
// with "possibly" semantics:  interval{ 3, 6 }  (not wrong but excessive)
// with "certainly" semantics:  interval{ 4, 5 }  (wrong)

auto maxCB = max(C, B);
// with "possibly" semantics:  interval{ 4, 5 }  (wrong)
// with "certainly" semantics:  interval{ 3, 6 }  (not wrong but excessive)
```

### Set-valued logic

As the root cause of our problem,
we identify the fact that the result of "𝑈 < 𝑉" can be ambiguous, and that this cannot be reflected
in the two-element Boolean algebra. However, we *can* represent ambiguity if we use the
[set extension](#set-and-interval-arithmetic) to define "𝑈 < 𝑉":

𝑈 < 𝑉 := { (𝑢 < 𝑣) │ 𝑢 ∈ 𝑈, 𝑣 ∈ 𝑉 } .

The resulting set is a *subset* of the two-element Boolean algebra with the set

𝔹 := { 𝚏𝚊𝚕𝚜𝚎, 𝚝𝚛𝚞𝚎 } .

In particular, if the intervals 𝑈 and 𝑉 overlap, then 𝑈 < 𝑉 = { 𝚏𝚊𝚕𝚜𝚎, 𝚝𝚛𝚞𝚎 }. Practically, if
our interval data type can represent an invalid state, which would semantically correspond to
the empty set, then 𝑈 < 𝑉 = ∅ if at least one of the arguments 𝑈 and 𝑉 is in an invalid state.
One could say that (𝑈 < 𝑉) is an element of the *powerset* of 𝔹,

𝒫(𝔹) = { ∅, { 𝚏𝚊𝚕𝚜𝚎 }, { 𝚝𝚛𝚞𝚎 }, { 𝚏𝚊𝚕𝚜𝚎, 𝚝𝚛𝚞𝚎 } } .

As it turns out, 𝒫(𝔹) is a [four-valued logic](https://en.wikipedia.org/wiki/Four-valued_logic):
the [logical connectives](https://en.wikipedia.org/wiki/Logical_connective) ∧, ∨, and ¬
can be given by the set extension,

𝐴 ∧ 𝐵 := { (𝑎 ∧ 𝑏) | 𝑎 ∈ 𝐴, 𝑏 ∈ 𝐵 } ,  
𝐴 ∨ 𝐵 := { (𝑎 ∨ 𝑏) | 𝑎 ∈ 𝐴, 𝑏 ∈ 𝐵 } ,  
¬𝐴 := { ¬𝑎 | 𝑎 ∈ 𝐴 } .

and the usual logical identities hold
([associativity](https://en.wikipedia.org/wiki/Associative_property#Propositional_logic),
[commutativity](https://en.wikipedia.org/wiki/Commutative_property#Propositional_logic),
[distributivity](https://en.wikipedia.org/wiki/Distributive_property#Propositional_logic),
[De Morgan’s laws](https://en.wikipedia.org/wiki/De_Morgan%27s_laws)).
Also, the system of relational predicates =, ≠, <, >, ≤, ≥ is
[logically consistent](#relational-predicates), so the usual relational identities can
be relied on without hesitation.

### Boolean projections

Because branches embody a dichotomy—to jump or not to jump—, branch conditions must necessarily
be Boolean values. Thus, if relational predicates between interval arguments are taken to be
set-valued, (𝑈 < 𝑉) ∈ 𝒫(𝔹) for intervals 𝑈 and 𝑉, then we need a way to interpret their
values as Boolean values. This can be accomplished with the following intuitive *projections*:

ᴘᴏssɪʙʟʏ: 𝒫(𝔹) → 𝔹, 𝑈 ↦ (𝚝𝚛𝚞𝚎 ∈ 𝑈) ,  
ᴀʟᴡᴀʏs: 𝒫(𝔹) → 𝔹, 𝑈 ↦ (𝚏𝚊𝚕𝚜𝚎 ∉ 𝑈) ,  
ᴄᴏɴᴛɪɴɢᴇɴᴛ: 𝒫(𝔹) → 𝔹, 𝑈 ↦ (𝑈 ≡ { 𝚏𝚊𝚕𝚜𝚎, 𝚝𝚛𝚞𝚎 }) ,  
ᴠᴀᴄᴜᴏᴜs: 𝒫(𝔹) → 𝔹, 𝑈 ↦ (𝑈 ≡ ∅) .

Among these projections, the following identities hold:

¬ᴀʟᴡᴀʏs(𝑈)  ⇔  ᴘᴏssɪʙʟʏ(¬𝑈) ,  
¬ᴘᴏssɪʙʟʏ(U)  ⇔  ᴀʟᴡᴀʏs(¬𝑈) ,  
ᴄᴏɴᴛɪɴɢᴇɴᴛ(U)  ⇔  ᴘᴏssɪʙʟʏ(U) ∧ ᴘᴏssɪʙʟʏ(¬U) ,  
ᴠᴀᴄᴜᴏᴜs(U)  ⇔  ᴀʟᴡᴀʏs(U) ∧ ᴀʟᴡᴀʏs(¬U) .

Using the C++ implementation of the ᴘᴏssɪʙʟʏ projection, we can now express the branch
conditions using set-valued predicates:

```c++
template <typename T>
T max(T a, T b)
{
    auto x = T{ };
    auto c = (a < b);  // ∈ 𝒫(𝔹)
    if (possibly(c))  // possibly(a < b)
    {
        x = b;
    }
    if (possibly(!c))  // possibly(!(a < b)) == possibly(a >= b)
    {
        x = a;
    }
    return x;
}
```

The projection functions have overloads for Boolean arguments; for example, if `c` is of
type `bool`, then `possibly(c) == c`. Thanks to these overloads, the `max()` function
template can be instantiated for both intervals or scalar types.

In the original `max()` function, the second branch was governed by an `else` clause,
equivalent to the negation of the preceding `if` clause. Here, this negation would be
`!possibly(c)`, which is clearly not equivalent to `possibly(!c)`. By keeping the relational
predicates set-valued and defining Boolean projections separately, the distinction between
"not possibly" and "possibly not" can now be represented correctly.


### Partial assignment

Depending on the values of the interval arguments `a` and `b` and on the relational semantics
chosen, we now have to consider two additional possibilities:
*neither branch* might be executed (if at least one argument is in the invalid state), or
*both branches* might be executed (if the argument intervals overlap)!

The first possibility can be accounted for by default-initializing the interval data type
to the invalid state. But handle the possibility of both branches being executed,
the assignment must be modified to avoid the second assignment spuriously overwriting the
first one.

To this end, *intervals* defines the function `assign_partial(x, a)` which widens the
interval `x` such that it encloses the interval argument `a`. To ensure that generic code
can be instantiated either for interval or for scalar arguments, `assign_partial(x, a)`
also has an overload for scalar arguments `x` and `a` which simply executes the assignment
`x = a`.

```c++
template <typename T>
T max2(T a, T b)
{
    auto x = T{ };
    auto c = (a < b);
    if (possibly(c))
    {
        assign_partial(x, b);
    }
    if (possibly(!c))
    {
        assign_partial(x, a);
    }
    return x;
}
```

With this modification, the second assignment to `x` will not overwrite but extend the first
assignment if both branches are executed. The interval instantiation of the `max2()`
function template now is an interval extension of the instantiation for scalar arguments.
However, the intervals returned can still be excessive. Consider again the examples of
max([2,4],[3,6]) = [3,6] and max([4,5],[3,6]) = [4,6]:

```c++
auto A = interval{ 2, 4 };
auto B = interval{ 4, 5 };
auto C = interval{ 3, 6 };

auto max2AC = max2(A, C);  //  interval{ 2, 6 }  (not wrong but excessive)
auto max2CA = max2(C, A);  // same

auto max2BC = max2(B, C);  //  interval{ 3, 6 }  (not wrong but excessive)
auto max2CB = max2(C, B);  // same
```

### Constraints

TODO

## Reference documentation

*intervals* mainly consists of the two class templates [**`interval<>`**](#interval) and [**`set<>`**](#set)
along with supporting infrastructure.

**Overview:**
- [`interval<>`](#interval)
- [`set<>`](#set)
- [Relational operators and constraints](#relational-operators-and-constraints)
- [Algorithms](#algorithms)
- [Utilities](#utilities)

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
- The **default constructor**, which creates an object with an invalid state:
  ```c++
  auto U = interval<int>{ };
  assert(!U.assigned());
  ```  
  The member function `U.assigned()` can be used to determine whether the interval has been
  assigned a value.  
  Note that the default constructor is not available for the specialization where `T` is a
  random-access iterator; this specialization has no invalid state.
- The **converting constructor**, which accepts an argument of the endpoint type `T`:
  ```c++
  auto V = interval{ 2 };  // represents degenerate interval [2, 2]
  assert(V.matches(2));
  ```
- The **endpoint constructor**, which accepts the two interval endpoints a and b of type `T`,
  where a ≤ b must hold:
  ```c++
  auto W = interval{ 0, 1 };  // represents interval [0, 1]
  //auto Z = interval{ 1, 0 };  // this would fail with a GSL precondition violation
  ```

#### Accessors

The bounds of an interval can be accessed directly or through a semantic accessor:
- For an object `X` of type `interval<T>`, the interval bounds can be determined as **`X.lower()`**
  and **`X.upper()`**:
  ```c++
  //U.lower();  // this would fail with a GSL precondition violation
  assert(V.lower() == V.upper());
  assert(W.lower() == 0);
  assert(W.upper() == 1);
  ```
- Alternatively, an interval can be decomposed into its bounds using
  [**structured bindings**](https://en.cppreference.com/w/cpp/language/structured_binding):
  ```c++
  auto [a, b] = W;
  assert(a == 0);
  assert(b == 1);
  ```
- To access the interval bounds without the runtime validity check, use **`X.lower_unchecked()`**
  and **`X.upper_unchecked()`**.
- The single value of a degenerate interval `X = interval{ x, x }` can be accessed with the member function
  **`X.value()`**:
  ```c++
  //U.value();  // this would fail with a GSL precondition violation
  assert(V.value() == 2);
  //W.value();  // this would fail with a GSL precondition violation
  ```
- To check whether an interval `Y = interval{ a, b }` *contains* another interval `Z = interval{ c, d }`
  of the same type – that is, whether 𝑐 ≥ 𝑎 ∧ 𝑑 ≤ 𝑏 –, the member function **`Y.contains()`** can be used:
  ```c++
  assert(interval{ 0., 3. }.contains(1.));
  assert(interval{ 0., 3. }.contains(interval{ 0., 1. }));
  assert(!interval{ 0., 3. }.contains(interval{ 1., 4. }));
  ```
- An interval `Y = interval{ a, b }` is said to *enclose* another interval `Z = interval{ c, d }` if
  𝑐 > 𝑎 ∧ 𝑑 < 𝑏, which can be checked with the member function **`Y.encloses()`**:
  ```c++
  assert(interval{ 0., 3. }.encloses(1.));
  assert(!interval{ 0., 3. }.encloses(0.));
  assert(interval{ 0., 3. }.encloses(interval{ 1., 2. }));
  assert(!interval{ 0., 2. }.encloses(interval{ 0., 1. }));
  assert(!interval{ 0., 2. }.encloses(interval{ -1., 1. }));
  ```
- An interval `Y = interval{ a, b }` *matches* another interval `Z = interval{ c, d }` if the endpoints
  of the intervals match exactly, 𝑐 = 𝑎 ∧ 𝑑 = 𝑏, which can be checked with the member function
  **`Y.matches()`**:
  ```c++
  assert(!interval{ 0., 3. }.matches(0.));
  assert(interval{ 0., 3. }.matches(interval{ 0., 3. }));
  assert(!interval{ 0., 2. }.matches(interval{ 0., 1. }));
  ```

TODO: `infimum()`, `supremum()`

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

The following mathematical functions are defined for `interval<T>` for a **floating-point** type argument `T`
as the [interval extension](#motivation) (precise unless indicated otherwise) of the respective real-valued
function:
- The unary arithmetic operators **`-`** and **`+`** and the binary arithmetic operators **`+`**,
  **`-`**, **`*`**, and **`/`**.
- **`min(U, V)`** and **`max(U, V)`**.
- **`square(U)`** and **`cube(U)`**, corresponding to the functions  
  𝑠𝑞𝑢𝑎𝑟𝑒: ℝ → ℝ, 𝑥 ↦ 𝑥²  
  and  
  𝑐𝑢𝑏𝑒: ℝ → ℝ, 𝑥 ↦ 𝑥³.
- [**`sqrt(U)`**](https://en.cppreference.com/w/cpp/numeric/math/sqrt) and
  [**`cbrt(U)`**](https://en.cppreference.com/w/cpp/numeric/math/cbrt), corresponding to the √ and ∛ functions.
- [**`abs(U)`**](https://en.cppreference.com/w/cpp/numeric/math/abs).
- [**`log(U)`**](https://en.cppreference.com/w/cpp/numeric/math/log),
  [**`exp(U)`**](https://en.cppreference.com/w/cpp/numeric/math/exp), and
  [**`pow(U, V)`**](https://en.cppreference.com/w/cpp/numeric/math/pow).
- The trigonometric functions [**`sin(U)`**](https://en.cppreference.com/w/cpp/numeric/math/sin),
  [**`cos(U)`**](https://en.cppreference.com/w/cpp/numeric/math/cos), and
  [**`tan(U)`**](https://en.cppreference.com/w/cpp/numeric/math/tan) and their inverses
  [**`asin(U)`**](https://en.cppreference.com/w/cpp/numeric/math/asin),
  [**`acos(U)`**](https://en.cppreference.com/w/cpp/numeric/math/acos), and
  [**`atan(U)`**](https://en.cppreference.com/w/cpp/numeric/math/atan).
- [**`atan2(V, U)`**](https://en.cppreference.com/w/cpp/numeric/math/atan2).
- [**`floor(U)`**](https://en.cppreference.com/w/cpp/numeric/math/floor),
  [**`ceil(U)`**](https://en.cppreference.com/w/cpp/numeric/math/ceil), and
  [**`round(U)`**](https://en.cppreference.com/w/cpp/numeric/math/round).
- **`frac(U)`**, corresponding to the function  
  𝑓𝑟𝑎𝑐: ℝ → ℝ, x ↦ x - floor(x).
- **`fractional_weights(U)`**, corresponding to the function  
  𝑓𝑟𝑎𝑐𝑡𝑖𝑜𝑛𝑎𝑙_𝑤𝑒𝑖𝑔ℎ𝑡𝑠: ℝ² → ℝ², (𝑎,𝑏) ↦ (𝑎/(𝑎 + 𝑏), 𝑏/(𝑎 + 𝑏)).
- **`blend_linear(U)`**, as the non-precise interval extension of the function  
  𝑏𝑙𝑒𝑛𝑑_𝑙𝑖𝑛𝑒𝑎𝑟: ℝ² × ℝ² → ℝ, ((𝑎,𝑏), (𝑥,𝑦)) ↦ 𝑎/(𝑎 + 𝑏)⋅x + 𝑏/(𝑎 + 𝑏)⋅y.
- **`blend_quadratic(U)`**, as the non-precise interval extension of the function  
  𝑏𝑙𝑒𝑛𝑑_𝑞𝑢𝑎𝑑𝑟𝑎𝑡𝑖𝑐: ℝ² × ℝ² → ℝ, ((𝑎,𝑏), (𝑥,𝑦)) ↦ √\[(𝑎/(𝑎 + 𝑏)⋅𝑥)² + (𝑏/(𝑎 + 𝑏)⋅𝑦)²\].

The following mathematical functions are defined for `interval<T>` for a **signed integral** type argument `T`
as the precise interval extension of the respective integer-valued function:
- The unary arithmetic operators **`-`** and **`+`** and the binary arithmetic operators **`+`**,
  **`-`**, **`*`**, and **`/`**.
- **`min(U, V)`** and **`max(U, V)`**.
- **`square(U)`** and **`cube(U)`**, corresponding to the functions  
  𝑠𝑞𝑢𝑎𝑟𝑒: ℤ → ℤ, 𝑥 ↦ 𝑥²  
  and  
  𝑐𝑢𝑏𝑒: ℤ → ℤ, 𝑥 ↦ 𝑥³.
- [**`abs(U)`**](https://en.cppreference.com/w/cpp/numeric/math/abs).

The following mathematical functions are defined for `interval<T>` for a random-access iterator type
argument `T` as the precise interval extension of the respective iterator-valued function:
- **`operator +(U, V)`** (offset iterator) where one argument is convertible to type `interval<T>`, and the
  other is an integer or an interval of integers.
- **`operator -(U, V)`** (offset iterator) where `U` is convertible to type `interval<T>`, and `V` is an
  integer or an interval of integers.
- **`operator -(U, V)`** (iterator difference) where both arguments are convertible to type `interval<T>`.
- **`prev(U)`** and **`next(U)`**.

#### Stream formatting

Intervals can be written to a stream using the **`<<`** operator. For degenerate intervals holding
only a single value, only the single endpoint is written.
```c++
//std::cout << U;  // this would fail with a GSL precondition violation
std::cout << "V = " << V << "\n";  // prints "V = 2"
std::cout << "W = " << W << "\n";  // prints "W = [0, 1]"
```

#### Conversion

Type conversions between different instantiations of `interval<>` are possible if the type arguments
are convertible. `interval<>` has an **implicit conversion constructor** for this purpose:
```c++
auto U = interval{ 0, 1 };  // `interval<int>`
interval<double> V = U;  // implicit conversion
```

Intervals can also be converted explicitly using one of the following casts inspired by the
[Guidelines Support Library](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#gslutil-utilities):
- **`narrow_cast<T>(U)`**, which creates an object of type `interval<T>` from the interval `U` by casting
  the lower and upper bounds to `T` with a `static_cast<>()`.
  ```c++
  auto U = interval{ 0., 0.5 };  // `interval<double>`
  auto V = narrow_cast<int>(U);  // explicit lossy conversion to `interval<int>{ 0 }`
  ```
- **`narrow_failfast<T>(U)`**, which behaves like `narrow_cast<T>(U)` but uses a GSL precondition
  check to assert that the value of `U` can be represented exactly by type `interval<T>`.
  ```c++
  auto U = interval{ 0., 0.5 };  // `interval<double>`
  auto V = narrow_failfast<float>(U);  // explicit conversion to `interval<float>{ 0.f, 0.5f }`
  //auto V = narrow_failfast<int>(U);  // this would fail with a GSL precondition violation
  ```
- **`narrow<T>(U)`**, which behaves like `narrow_cast<T>(U)` but throws a `gsl_lite::narrowing_error`
  exception if the value of `U` cannot be represented exactly by type `interval<T>`.
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
- `constrain()`

For arguments `b` convertible to `bool` and `c` convertible to `set<bool>`, *intervals*
defines the following overloads of the [Boolean projections](#boolean-projections):

- **`possibly(b)`**, returning `b`, and **`possibly(c)`**, returning `c.contains(true)`;
- **`possibly_not(b)`**, returning `!b`, and **`possibly_not(c)`**, returning `c.contains(false)`;
- **`always(b)`**, returning `b`, and **`always(c)`**, returning `!c.contains(false)`;
- **`never(b)`**, returning `!b`, and **`never(c)`**, returning `!c.contains(true)`;
- **`contingent(b)`**, returning `false`, and **`contingent(c)`**, returning `c.matches(set{ false, true })`;
- **`vacuous(b)`**, returning `false`, and **`vacuous(c)`**, returning `c.matches(set<bool>{ })`.

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
- interval-related concepts


## Other implementations

Many other libraries for interval arithmetics exist. Some exemplary C++ libraries are
[Boost Interval Arithmetic Library](https://www.boost.org/doc/libs/1_81_0/libs/numeric/interval/doc/interval.htm)
and [GAOL](https://frederic.goualard.net/#research-software-gaol).

*intervals* differs from other libraries mostly by its handling of relational comparison operators,
encouraging a particular style of interval-aware programming.


## References

The *intervals* library has been motivated and introduced in the following CoNGA 2023 conference paper:

[1]  Beutel, M. & Strzodka, R. 2023, *A paradigm for interval-aware programming*, in Next Generation Arithmetic, ed. J. Gustafson, S. H.
Leong, & M. Michalewicz, Lecture Notes in Computer Science (Cham: Springer Nature Switzerland), 38–60  
https://link.springer.com/chapter/10.1007/978-3-031-32180-1_3

Other references:

[2] *IEEE Standard for Interval Arithmetic.* IEEE Std 1788-2015 pp. 1–97 (Jun 2015).  
https://doi.org/10.1109/IEEESTD.2015.7140721

[3] Hickey, T., Ju, Q., Van Emden, M.H.: *Interval arithmetic: From principles to
implementation.* Journal of the ACM 48(5), 1038–1068 (Sep 2001).  
https://doi.org/10.1145/502102.502106
