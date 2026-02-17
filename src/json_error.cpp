#include "json_error.hpp"
#include <string>

namespace jansson {

const char* JsonErrorCategory::name() const noexcept {
    return "json";
}

std::string JsonErrorCategory::message(int ev) const {
    switch (static_cast<JsonErrorCode>(ev)) {
        case JsonErrorCode::Success:
            return "Success";
        case JsonErrorCode::MemoryAllocationFailed:
            return "Memory allocation failed";
        case JsonErrorCode::InvalidUTF8:
            return "Invalid UTF-8 sequence";
        case JsonErrorCode::SyntaxError:
            return "JSON syntax error";
        case JsonErrorCode::InvalidType:
            return "Invalid type";
        case JsonErrorCode::KeyNotFound:
            return "Key not found";
        case JsonErrorCode::IndexOutOfBounds:
            return "Index out of bounds";
        case JsonErrorCode::InvalidArgument:
            return "Invalid argument";
        case JsonErrorCode::ParseError:
            return "Parse error";
        case JsonErrorCode::SerializationError:
            return "Serialization error";
        case JsonErrorCode::NotImplemented:
            return "Not implemented";
        case JsonErrorCode::UnknownError:
            return "Unknown error";
        default:
            return "Unknown JSON error";
    }
}

const std::error_category& json_error_category() noexcept {
    static JsonErrorCategory category;
    return category;
}

std::error_code make_error_code(JsonErrorCode code) noexcept {
    return std::error_code(static_cast<int>(code), json_error_category());
}

} // namespace jansson
