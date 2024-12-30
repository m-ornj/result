#include <catch2/catch_test_macros.hpp>
#include <result/result.hpp>

using namespace result;

TEST_CASE("Basic Construction", "[Result]") {
    SECTION("Construct with Ok") {
        Result<int, std::string> result_ok(Ok<int> {42});
        REQUIRE(!result_ok.has_error());
        REQUIRE(result_ok.unwrap() == 42);
    }

    SECTION("Construct with Error") {
        Result<int, std::string> result_error(Error<std::string> {"Error occurred"});
        REQUIRE(result_error.has_error());
        REQUIRE(result_error.error() == "Error occurred");
    }
}

TEST_CASE("Void Specialization", "[Result]") {
    SECTION("OkType = void") {
        Result<void, std::string> result_ok(Ok<void> {});
        REQUIRE(!result_ok.has_error());
    }

    SECTION("Error for void specialization") {
        Result<void, std::string> result_error(Error<std::string> {"Void Error"});
        REQUIRE(result_error.has_error());
        REQUIRE(result_error.error() == "Void Error");
    }
}

TEST_CASE("Copy and Move Semantics", "[Result]") {
    SECTION("Copy construction and assignment") {
        Result<int, std::string> original_ok(Ok<int> {42});
        Result<int, std::string> copy_ok = original_ok;
        REQUIRE(copy_ok.unwrap() == 42);

        Result<int, std::string> original_error(Error<std::string> {"Error"});
        Result<int, std::string> copy_error = original_error;
        REQUIRE(copy_error.error() == "Error");
    }

    SECTION("Move construction and assignment") {
        Result<int, std::string> original_ok(Ok<int> {42});
        Result<int, std::string> moved_ok = std::move(original_ok);
        REQUIRE(moved_ok.unwrap() == 42);

        Result<int, std::string> original_error(Error<std::string> {"Error"});
        Result<int, std::string> moved_error = std::move(original_error);
        REQUIRE(moved_error.error() == "Error");
    }
}

TEST_CASE("Unwrap Method", "[Result]") {
    SECTION("Unwrap valid result") {
        Result<int, std::string> result_ok(Ok<int> {42});
        REQUIRE(result_ok.unwrap() == 42);
    }

    SECTION("Unwrap throws on error") {
        Result<int, std::string> result_error(Error<std::string> {"Cannot unwrap"});
        REQUIRE_THROWS(result_error.unwrap());
    }
}

TEST_CASE("Error Method", "[Result]") {
    SECTION("Error for error result") {
        Result<int, std::string> result_error(Error<std::string> {"Error occurred"});
        REQUIRE(result_error.error() == "Error occurred");
    }

    SECTION("Error throws on valid result") {
        Result<int, std::string> result_ok(Ok<int> {42});
        REQUIRE_THROWS(result_ok.error());
    }
}

TEST_CASE("Map Method", "[Result]") {
    SECTION("Map transforms OkType") {
        Result<int, std::string> result_ok(Ok<int> {42});
        auto mapped = result_ok.map([](int x) { return x + 1; });
        REQUIRE(mapped.unwrap() == 43);
    }

    SECTION("Map does not transform error result") {
        Result<int, std::string> result_error(Error<std::string> {"Error occurred"});
        auto mapped_error = result_error.map([](int x) { return x + 1; });
        REQUIRE(mapped_error.has_error());
        REQUIRE(mapped_error.error() == "Error occurred");
    }
}

TEST_CASE("Map Error Method", "[Result]") {
    SECTION("Map error transforms ErrorType") {
        Result<int, std::string> result_error(Error<std::string> {"Error occurred"});
        auto mapped = result_error.map_error([](const std::string& e) { return "Mapped: " + e; });
        REQUIRE(mapped.has_error());
        REQUIRE(mapped.error() == "Mapped: Error occurred");
    }

    SECTION("Map error does not transform valid result") {
        Result<int, std::string> result_ok(Ok<int> {42});
        auto unchanged_ok = result_ok.map_error([](const std::string& e) { return "Mapped: " + e; });
        REQUIRE(!unchanged_ok.has_error());
        REQUIRE(unchanged_ok.unwrap() == 42);
    }
}

TEST_CASE("Void Map and Map Error", "[Result]") {
    SECTION("Map for OkType = void") {
        Result<void, std::string> result_ok(Ok<void> {});
        auto mapped = result_ok.map([] { return 42; });
        REQUIRE(!mapped.has_error());
        REQUIRE(mapped.unwrap() == 42);
    }

    SECTION("Map error for OkType = void") {
        Result<void, std::string> result_error(Error<std::string> {"Error"});
        auto mapped_error = result_error.map_error([](const std::string& e) { return e + " mapped"; });
        REQUIRE(mapped_error.has_error());
        REQUIRE(mapped_error.error() == "Error mapped");
    }
}

TEST_CASE("Chaining and Composability", "[Result]") {
    SECTION("Chained map calls") {
        Result<int, std::string> result_ok(Ok<int> {42});
        auto chained = result_ok
        .map([](int x) { return x * 2; })
        .map([](int x) { return x - 10; });
        REQUIRE(chained.unwrap() == 74);
    }

    SECTION("Chained map_error calls") {
        Result<int, std::string> result_error(Error<std::string> {"Initial error"});
        auto chained_error = result_error
        .map([](int x) { return x * 2; })
        .map_error([](const std::string& e) { return e + " mapped"; });
        REQUIRE(chained_error.has_error());
        REQUIRE(chained_error.error() == "Initial error mapped");
    }
}


struct NonCopyable {
    int value;
    NonCopyable(int v) : value(v) {}
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) noexcept = default;
    NonCopyable& operator=(NonCopyable&&) noexcept = default;
};

TEST_CASE("Result with non-copyable OkType", "[Result]") {
    SECTION("Move-only construction") {
        Result<NonCopyable, std::string> result_ok(Ok<NonCopyable> {NonCopyable(42)});
        REQUIRE(!result_ok.has_error());
        REQUIRE(result_ok.unwrap().value == 42);
        NonCopyable ok = std::move(result_ok).unwrap();
        REQUIRE(ok.value == 42);
    }

    SECTION("Move-only assignment") {
        Result<NonCopyable, std::string> result_ok(Ok<NonCopyable> {NonCopyable(42)});
        Result<NonCopyable, std::string> moved_ok = std::move(result_ok);
        REQUIRE(!moved_ok.has_error());
        REQUIRE(moved_ok.unwrap().value == 42);
        NonCopyable ok = std::move(moved_ok).unwrap();
        REQUIRE(ok.value == 42);
    }

    SECTION("Map with move-only OkType") {
        Result<NonCopyable, std::string> result_ok(Ok<NonCopyable> {NonCopyable(42)});
        auto mapped = std::move(result_ok).map([](NonCopyable&& nc) { return nc.value * 2; });
        REQUIRE(!mapped.has_error());
        REQUIRE(mapped.unwrap() == 84);
        NonCopyable ok = std::move(mapped).unwrap();
        REQUIRE(ok.value == 84);
    }

    SECTION("Chaining with move-only types") {
        Result<NonCopyable, std::string> result_ok(Ok<NonCopyable> {NonCopyable(42)});
        auto chained = std::move(result_ok)
        .map([](NonCopyable nc) { return NonCopyable(nc.value * 2); })
        .map([](NonCopyable nc) { return NonCopyable(nc.value + 10); });
        REQUIRE(!chained.has_error());
        REQUIRE(chained.unwrap().value == 94);
        NonCopyable ok = std::move(chained).unwrap();
        REQUIRE(ok.value == 94);
    }
}

TEST_CASE("Result with non-copyable ErrorType", "[Result]") {
    SECTION("Move-only error construction") {
        Result<int, NonCopyable> result_error(Error<NonCopyable> {NonCopyable(42)});
        REQUIRE(result_error.has_error());
        REQUIRE(result_error.error().value == 42);
        NonCopyable error = std::move(result_error).error();
        REQUIRE(error.value == 42);
    }

    SECTION("Move-only error assignment") {
        Result<int, NonCopyable> result_error(Error<NonCopyable> {NonCopyable(42)});
        Result<int, NonCopyable> moved_error = std::move(result_error);
        REQUIRE(moved_error.has_error());
        REQUIRE(moved_error.error().value == 42);
        NonCopyable error = std::move(moved_error).error();
        REQUIRE(error.value == 42);
    }

    SECTION("Map error with move-only ErrorType") {
        Result<int, NonCopyable> result_error(Error<NonCopyable> {NonCopyable(42)});
        auto mapped_error = std::move(result_error).map_error([](NonCopyable nc) { return nc.value + 10; });
        REQUIRE(mapped_error.has_error());
        REQUIRE(mapped_error.error() == 52);
        NonCopyable error = std::move(mapped_error).error();
        REQUIRE(error.value == 52);
    }

    SECTION("Void specialization with move-only ErrorType") {
        Result<void, NonCopyable> result_error(Error<NonCopyable> {NonCopyable(42)});
        REQUIRE(result_error.has_error());
        REQUIRE(result_error.error().value == 42);
        NonCopyable error = std::move(result_error).error();
        REQUIRE(error.value == 42);
    }
}

constexpr Result<int, const char*> create_ok() { return Ok<int> {42}; }
constexpr Result<int, const char*> create_error() { return Error<const char*> {"Error"}; }

TEST_CASE("Constexpr Result", "[Result]") {
    constexpr auto result_ok = create_ok();
    STATIC_REQUIRE(!result_ok.has_error());
    STATIC_REQUIRE(result_ok.unwrap() == 42);

    constexpr auto result_error = create_error();
    STATIC_REQUIRE(result_error.has_error());
    STATIC_REQUIRE(std::string(result_error.error()) == "Error");
}

struct EmptyType {};

TEST_CASE("Result with empty types", "[Result]") {
    SECTION("OkType as empty struct") {
        Result<EmptyType, std::string> result_ok(Ok<EmptyType> {EmptyType{}});
        REQUIRE(!result_ok.has_error());
        REQUIRE_NOTHROW(result_ok.unwrap());
    }

    SECTION("ErrorType as empty struct") {
        Result<int, EmptyType> result_error(Error<EmptyType> {EmptyType{}});
        REQUIRE(result_error.has_error());
        REQUIRE_NOTHROW(result_error.error());
    }
}

TEST_CASE("Result with recursive types", "[Result]") {
    using RecursiveResult = Result<Result<int, std::string>, std::string>;

    SECTION("Nested OkType") {
        RecursiveResult result(Ok<Result<int, std::string>> {Ok<int>{42}});
        REQUIRE(!result.has_error());
        REQUIRE(result.unwrap().unwrap() == 42);
    }

    SECTION("Nested ErrorType") {
        RecursiveResult result(Error<std::string> {"Outer error"});
        REQUIRE(result.has_error());
        REQUIRE(result.error() == "Outer error");
    }
}

TEST_CASE("Result with std::function as types", "[Result]") {
    SECTION("OkType as std::function") {
        Result<std::function<int(int)>, std::string> result_ok(Ok<std::function<int(int)>> {
            [](int x) { return x * 2; }
        });
        REQUIRE(!result_ok.has_error());
        REQUIRE(result_ok.unwrap()(21) == 42);
    }

    SECTION("ErrorType as std::function") {
        Result<int, std::function<std::string()>> result_error(Error<std::function<std::string()>> {
            []() { return "Error occurred"; }
        });
        REQUIRE(result_error.has_error());
        REQUIRE(result_error.error()() == "Error occurred");
    }
}

TEST_CASE("Deeply nested Result", "[Result]") {
    using DeepResult = Result<Result<Result<int, std::string>, std::string>, std::string>;

    SECTION("Unwrap deeply nested Ok") {
        DeepResult result(Ok<Result<Result<int, std::string>, std::string>> {
            Ok<Result<int, std::string>>{Ok<int>{42}}
        });
        REQUIRE(!result.has_error());
        REQUIRE(result.unwrap().unwrap().unwrap() == 42);
    }

    SECTION("Error at outermost level") {
        DeepResult result(Error<std::string> {"Outer error"});
        REQUIRE(result.has_error());
        REQUIRE(result.error() == "Outer error");
    }

    SECTION("Error at middle level") {
        DeepResult result(Ok<Result<Result<int, std::string>, std::string>> {
            Error<std::string>{"Middle error"}
        });
        REQUIRE(result.unwrap().has_error());
        REQUIRE(result.unwrap().error() == "Middle error");
    }
}
