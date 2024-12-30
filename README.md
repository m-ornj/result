# Result

A lightweight, single-header C++ library for error handling inspired by patterns from languages like Rust and Swift.

### Features

- Single-Header: Just one file, result.hpp.
- Dependency-Free: No external libraries required.
- Modern C++: Built with C++20 concepts and features.
- Lightweight: Minimal overhead and easy to integrate.

### Usage
Basic usage with a straightforward success/error scenario:
```cpp
#include <iostream>
#include <result/result.hpp>

using namespace result;

Result<int, std::string> safe_divide(int numerator, int denominator) {
    if (denominator == 0) {
        return Error{std::string{"Division by zero"}};
    }
    return Ok{numerator / denominator};
}

int main() {
    auto result = safe_divide(10, 0);

    if (result.has_error()) {
        std::cerr << "Error: " << result.error() << std::endl;
    } else {
        std::cout << "Result: " << result.unwrap() << std::endl;
    }

    return 0;
}
```

Using ```map``` to process the success value and ```map_error``` to transform the error into a user-friendly format:
```cpp
#include <iostream>
#include <result/result.hpp>

using namespace result;

enum class FetchError {
    ConnectionFailed,
    Timeout,
    InvalidResponse
};

Result<std::string, FetchError> fetch_data(std::string_view url) {
    if (url.empty()) {
        return Error{FetchError::ConnectionFailed};
    }
    if (url == "timeout") {
        return Error{FetchError::Timeout};
    }
    if (url == "invalid") {
        return Error{FetchError::InvalidResponse};
    }
    return Ok{std::string{"Fake data"}};
}

// Map FetchError to user-friendly strings
std::string fetch_error_to_string(FetchError error) {
    switch (error) {
    case FetchError::ConnectionFailed: return "Unable to connect";
    case FetchError::Timeout: return "Request timed out";
    case FetchError::InvalidResponse: return "Invalid response";
    }
    return "Unknown error";
}

int main() {
    auto result = fetch_data("invalid")
        .map([](std::string data) {
            return data + " (processed)";
        })
        .map_error([](FetchError error) {
            return fetch_error_to_string(error);
        });

    if (result.has_error()) {
        std::cerr << "Error: " << result.error() << std::endl;
    } else {
        std::cout << "Success: " << result.unwrap() << std::endl;
    }

    return 0;
}
```
