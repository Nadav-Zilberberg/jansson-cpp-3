#ifndef JSON_ERROR_HPP
#define JSON_ERROR_HPP

#include <system_error>
#include <stdexcept>
#include <string>
#include <variant>

namespace jansson {

// Error codes for JSON operations
enum class JsonErrorCode {
    Success = 0,
    MemoryAllocationFailed,
    InvalidUTF8,
    SyntaxError,
    InvalidType,
    KeyNotFound,
    IndexOutOfBounds,
    InvalidArgument,
    ParseError,
    SerializationError,
    NotImplemented,
    UnknownError
};

// Error category for JSON errors
class JsonErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override;
    std::string message(int ev) const override;
};

// Get the JSON error category
const std::error_category& json_error_category() noexcept;

// JSON exception base class
class JsonException : public std::runtime_error {
public:
    explicit JsonException(const std::string& message)
        : std::runtime_error(message) {}
    explicit JsonException(const char* message)
        : std::runtime_error(message) {}
};

// Result type for functional-style error handling
template <typename T>
class Result {
public:
    // Construct with value
    explicit Result(T value) : data_(std::move(value)) {}
    
    // Construct with error
    explicit Result(std::error_code ec) : data_(ec) {}
    
    // Check if result contains a value
    explicit operator bool() const { 
        return std::holds_alternative<T>(data_); 
    }
    
    // Get the value (throws if error)
    T& value() &
    {
        if (auto* val = std::get_if<T>(&data_)) {
            return *val;
        }
        throw JsonException(std::get<std::error_code>(data_).message());
    }
    
    const T& value() const &
    {
        if (const auto* val = std::get_if<T>(&data_)) {
            return *val;
        }
        throw JsonException(std::get<std::error_code>(data_).message());
    }
    
    // Get the error code
    std::error_code error() const
    {
        if (const auto* ec = std::get_if<std::error_code>(&data_)) {
            return *ec;
        }
        return std::error_code(static_cast<int>(JsonErrorCode::Success), json_error_category());
    }
    
    // Map the value
    template <typename F>
    auto map(F&& f) -> Result<decltype(f(std::declval<T>()))>
    {
        if (auto* val = std::get_if<T>(&data_)) {
            return Result<decltype(f(*val))>(f(*val));
        }
        return Result<decltype(f(std::declval<T>()))>(std::get<std::error_code>(data_));
    }
    
    // And then (monadic bind)
    template <typename F>
    auto and_then(F&& f) -> decltype(f(std::declval<T>()))
    {
        if (auto* val = std::get_if<T>(&data_)) {
            return f(*val);
        }
        return decltype(f(std::declval<T>()))(std::get<std::error_code>(data_));
    }

private:
    std::variant<T, std::error_code> data_;
};

// Helper function to create error codes
std::error_code make_error_code(JsonErrorCode code) noexcept;

} // namespace jansson

// Register JsonErrorCode with std::error_code
namespace std {
    template<> 
    struct is_error_code_enum<jansson::JsonErrorCode> : true_type {};
}

#endif // JSON_ERROR_HPP
