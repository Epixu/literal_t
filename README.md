### One template argument to rule them all!

Are you tired of not being able to set template arguments to string literals?<br>
Are you tired of every possible workaround leading to tons of boilerplate and increased build times?<br>
Well... today's your lucky day!

#### Check this out:
```c++
#include <Langulus/Literal.hpp>

using namespace Langulus;

template<literal_t T = 0>
consteval auto foo() { return T; }

constexpr auto demo1 = foo<>();
constexpr auto demo2 = foo<5>();
constexpr auto demo3 = foo<5.5f>();
constexpr auto demo4 = foo<true>();

// ...*drum roll*...

constexpr auto demo5 = foo<"hello there">();
```

-----------------

✅ C++23<br>
✅ Fully supporting `constexpr`/`consteval`<br>
✅ Will throw on range errors when `LANGULUS_OPTION_SAFE_MODE` is defined<br>
✅ Tested on `Clang 19`, `GCC 14.2`, ~~`MSVC v143`~~, `Clang-CL 19`<br>

> [!CAUTION]
> Not supported on MSVC yet (as of 11.2025).
> They have a lot of catching up to do with C++23 in order for this to work properly on their compiler.
-----------------

`literal_t` acts as both a single value, or string literal. You can use it as a template parameter.
Similar implementation should be introduced in C++26 as [`std::fixed_string`](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p3094r0.html), supposedly...
                                                                       
String literals of different sizes result in unique types, and thus can't often be used with ternary operators,
so I've taken the liberty to allow for strings of the form `? "\0\0\0" : "alt"` - left `literal_t` has `ArraySize == 4`, but `size() == 0`.
So you can do fun stuff at compile time, like, like:
```c++
constexpr literal_t Name = same_as<int32_t, int> ? "i\0\0" : "i32";
```

This also allows us to do some neat compile speed/memory optimizations by minimizing template 
instantiation via CTAD.
`literal_t`'s template will always instantiate with `ArraySize` being a power-of-two, regardless of the string's null-terminated length.

-----------------

### Getting it:
```cmake
include(FetchContent)
FetchContent_Declare(LangulusLiteral
    GIT_REPOSITORY  https://github.com/Epixu/literal_t.git
    GIT_TAG         main
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(LangulusLiteral)

target_link_libraries(YourTarget PUBLIC LangulusLiteral)
```
Alternatively, you can always just copy the `include/Langulus/Literal.hpp` header, if you prefer to keep it simple.
