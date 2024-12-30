#pragma once

#include <optional>
#include <string>
#include <variant>

namespace result {
    template <typename T = void>
    struct Ok {
        constexpr Ok(T v) noexcept : value(std::move(v)) {}
        T value;
    };

    template <>
    struct Ok<void> {};

    template <typename T>
    struct Error {
        constexpr Error(T v) noexcept : value(std::move(v)) {}
        T value;
    };

    template <typename T>
    struct ErrorDescription;

    template <typename T>
    concept HasErrorDescription = requires(T error) {
        { ErrorDescription<T>::description(error) } -> std::convertible_to<std::string_view>;
    };

    template <typename T>
    class BadUnwrapException : public std::exception {
    public:
        BadUnwrapException(T e) : m_error(std::move(e)) { construct_message(); }
        const char* what() const noexcept override { return m_message.c_str(); }

    private:
        void construct_message() requires(HasErrorDescription<T>) {
            m_message = "Failed to unwrap Result: ";
            m_message.append(ErrorDescription<T>::description(m_error));
        }

        void construct_message() requires(!HasErrorDescription<T>) {
            m_message = "Failed to unwrap Result";
        }

    private:
        T m_error;
        std::string m_message;
    };

    template <typename OkType, typename ErrorType>
    class Result {
    public:
        constexpr Result(Ok<OkType> v) : m_value(std::in_place_index<OkIndex>, std::move(v.value)) {}
        constexpr Result(Error<ErrorType> v) : m_value(std::in_place_index<ErrorIndex>, std::move(v.value)) {}

        constexpr Result& operator=(Ok<OkType> v) {
            m_value = std::variant<OkType, ErrorType> {std::in_place_index<OkIndex>, std::move(v.value)};
            return *this;
        }

        constexpr Result& operator=(Error<ErrorType> v) {
            m_value = std::variant<OkType, ErrorType> {std::in_place_index<ErrorIndex>, std::move(v.value)};
            return *this;
        }

        [[nodiscard]] constexpr const OkType& unwrap() const& {
            if (const ErrorType* error_ptr = std::get_if<ErrorIndex>(&m_value)) {
                throw BadUnwrapException<ErrorType>(*error_ptr);
            }
            const OkType* ok_ptr = std::get_if<OkIndex>(&m_value);
            return *ok_ptr;
        }

        [[nodiscard]] constexpr OkType&& unwrap() && {
            if (ErrorType* error_ptr = std::get_if<ErrorIndex>(&m_value)) {
                throw BadUnwrapException<ErrorType>(std::move(*error_ptr));
            }
            OkType* ok_ptr = std::get_if<OkIndex>(&m_value);
            return std::move(*ok_ptr);
        }

        [[nodiscard]] constexpr const ErrorType& error() const& {
            if (const OkType* ok_ptr = std::get_if<OkIndex>(&m_value)) {
                throw std::runtime_error("Failed to access error of a successful Result");
            }
            const ErrorType* error_ptr = std::get_if<ErrorIndex>(&m_value);
            return *error_ptr;
        }

        [[nodiscard]] constexpr ErrorType&& error() && {
            if (const OkType* ok_ptr = std::get_if<OkIndex>(&m_value)) {
                throw std::runtime_error("Failed to access error of a successful Result");
            }
            ErrorType* error_ptr = std::get_if<ErrorIndex>(&m_value);
            return std::move(*error_ptr);
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return std::get_if<OkIndex>(&m_value); }
        [[nodiscard]] constexpr bool has_error() const noexcept { return std::get_if<ErrorIndex>(&m_value); }
        constexpr operator bool() const noexcept { return has_value(); }

        template <typename F, typename NewOkType = std::invoke_result_t<F, const OkType&>>
        [[nodiscard]] constexpr auto map(F f) const & noexcept(std::is_nothrow_invocable_v<F, const OkType&>)
        -> Result<NewOkType, ErrorType> {
            if (has_error()) {
                return Error{error()};
            }
            if constexpr (std::is_same_v<NewOkType, void>) {
                f(unwrap());
                return Ok<void> {};
            } else {
                return Ok{f(unwrap())};
            }
        }

        template <typename F, typename NewOkType = std::invoke_result_t<F, OkType&&>>
        [[nodiscard]] constexpr auto map(F f) && noexcept(std::is_nothrow_invocable_v<F, OkType&&>)
        -> Result<NewOkType, ErrorType> {
            if (has_error()) {
                return Error{std::move(*this).error()};
            }
            if constexpr (std::is_same_v<NewOkType, void>) {
                f(f(std::move(*this).unwrap()));
                return Ok<void> {};
            } else {
                return Ok{f(std::move(*this).unwrap())};
            }
        }

        template <typename F, typename NewErrorType = std::invoke_result_t<F, const ErrorType&>>
        [[nodiscard]] constexpr auto map_error(F f) const & noexcept(std::is_nothrow_invocable_v<F, const ErrorType&>)
        -> Result<OkType, NewErrorType> {
            if (has_value()) {
                return Ok{unwrap()};
            }
            return Error{f(error())};
        }

        template <typename F, typename NewErrorType = std::invoke_result_t<F, ErrorType>>
        [[nodiscard]] constexpr auto map_error(F f) && noexcept(std::is_nothrow_invocable_v<F, ErrorType>)
        -> Result<OkType, NewErrorType> {
            if (has_value()) {
                return Ok{std::move(*this).unwrap()};
            }
            return Error{f(std::move(*this).error())};
        }

    private:
        constexpr static inline std::size_t OkIndex = 0;
        constexpr static inline std::size_t ErrorIndex = 1;

        std::variant<OkType, ErrorType> m_value;
    };

    template <typename ErrorType>
    class Result<void, ErrorType> {
    public:
        constexpr Result(Ok<>) : m_error(std::nullopt) {}
        constexpr Result(Error<ErrorType> v) : m_error(std::move(v.value)) {}

        constexpr Result& operator=(Ok<>) {
            m_error = std::nullopt;
            return *this;
        }

        constexpr Result& operator=(Error<ErrorType> v) {
            m_error = std::move(v.value);
            return *this;
        }

        constexpr void unwrap() const& {
            if (m_error.has_value()) {
                throw BadUnwrapException<ErrorType>(m_error.value());
            }
        }

        constexpr void unwrap() && {
            if (m_error.has_value()) {
                throw BadUnwrapException<ErrorType>(std::move(m_error).value());
            }
        }

        [[nodiscard]] constexpr const ErrorType& error() const& {
            if (m_error == std::nullopt) {
                throw std::runtime_error("Failed to access error of a successful Result");
            }
            return m_error.value();
        }

        [[nodiscard]] constexpr ErrorType&& error() && {
            if (m_error == std::nullopt) {
                throw std::runtime_error("Failed to access error of a successful Result");
            }
            return std::move(m_error).value();
        }

        [[nodiscard]] constexpr bool has_error() const noexcept { return m_error.has_value(); }
        constexpr operator bool() const noexcept { return !has_error(); }

        template <typename F, typename NewOkType = std::invoke_result_t<F>>
        [[nodiscard]] constexpr auto map(F f) const & noexcept(std::is_nothrow_invocable_v<F>)
        -> Result<NewOkType, ErrorType> {
            if (m_error.has_value()) {
                return Error{m_error.value()};
            }
            if constexpr (std::is_same_v<NewOkType, void>) {
                f();
                return Ok<void> {};
            } else {
                return Ok{f()};
            }
        }

        template <typename F, typename NewOkType = std::invoke_result_t<F>>
        [[nodiscard]] constexpr auto map(F f) && noexcept(std::is_nothrow_invocable_v<F>)
        -> Result<NewOkType, ErrorType> {
            if (m_error.has_value()) {
                return Error{std::move(m_error).value()};
            }
            if constexpr (std::is_same_v<NewOkType, void>) {
                f();
                return Ok<void> {};
            } else {
                return Ok{f()};
            }
        }

        template <typename F, typename NewErrorType = std::invoke_result_t<F, const ErrorType&>>
        [[nodiscard]] constexpr auto map_error(F f) const & noexcept(std::is_nothrow_invocable_v<F, const ErrorType&>)
        -> Result<void, NewErrorType> {
            if (m_error == std::nullopt) {
                return Ok<void> {};
            }
            return Error{f(m_error.value())};
        }

        template <typename F, typename NewErrorType = std::invoke_result_t<F, ErrorType>>
        [[nodiscard]] constexpr auto map_error(F f) && noexcept(std::is_nothrow_invocable_v<F, ErrorType>)
        -> Result<void, NewErrorType> {
            if (m_error == std::nullopt) {
                return Ok<void> {};
            }
            return Error{f(std::move(m_error).value())};
        }

    private:
        std::optional<ErrorType> m_error;
    };
}
