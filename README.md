# Custom Expected

A from-scratch implementation of a C++23-style `expected` type written in modern C++.

Provides a type-safe way to represent either a successful value or an error, with monadic operations.

## Features

- `expected<Value, Error>` with union storage
- `unexpected` helper
- Monadic operations: `and_then`, `transform`, `or_else`
- Full support for lvalue / rvalue / const overloads
- Constexpr-friendly

## Example

```cpp
constexpr expected<int, const char*> Foo(int num)
{
    if (num > 0)
        return num;

    return unexpected{"67"};
}

auto e = Foo(-1)
    .transform([](int n) { return n / 2; })
    .and_then([](int n) -> expected<int, const char*> {
        return n * 2;
    });
```

## Build

This is a Visual Studio project. Open `ExpectedFromScratch.sln` or compile the files manually with a C++20/23 compiler.
