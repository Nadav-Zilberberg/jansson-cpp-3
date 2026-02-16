#include "string_utils.hpp"
#include "json_error.hpp"
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace jansson {

JsonString::JsonString(std::string value)
    : value_(std::move(value))
{
    if (!validate_utf8(value_)) {
        throw JsonException("Invalid UTF-8 sequence in JsonString");
    }
}

JsonString::JsonString(std::string_view value)
    : value_(value)
{
    if (!validate_utf8(value_)) {
        throw JsonException("Invalid UTF-8 sequence in JsonString");
    }
}

bool JsonString::operator==(const JsonString& other) const noexcept {
    return value_ == other.value_;
}

bool JsonString::operator!=(const JsonString& other) const noexcept {
    return value_ != other.value_;
}

bool JsonString::operator<(const JsonString& other) const noexcept {
    return value_ < other.value_;
}

bool JsonString::validate_utf8(std::string_view input) noexcept {
    const uint8_t* data = reinterpret_cast<const uint8_t*>(input.data());
    size_t length = input.length();
    size_t i = 0;
    
    while (i < length) {
        uint8_t byte = data[i];
        
        // 1-byte sequence (0x00-0x7F)
        if ((byte & 0x80) == 0x00) {
            i++;
            continue;
        }
        
        // Determine number of bytes in sequence
        int num_bytes = 0;
        if ((byte & 0xE0) == 0xC0) {
            num_bytes = 2;
        } else if ((byte & 0xF0) == 0xE0) {
            num_bytes = 3;
        } else if ((byte & 0xF8) == 0xF0) {
            num_bytes = 4;
        } else {
            // Invalid leading byte
            return false;
        }
        
        // Check if we have enough bytes
        if (i + num_bytes > length) {
            return false;
        }
        
        // Check continuation bytes
        for (int j = 1; j < num_bytes; ++j) {
            if ((data[i + j] & 0xC0) != 0x80) {
                return false;
            }
        }
        
        // Validate code point
        uint32_t code_point = 0;
        if (num_bytes == 2) {
            code_point = (byte & 0x1F) << 6;
            code_point |= (data[i + 1] & 0x3F);
            if (code_point < 0x80) {
                return false; // Overlong encoding
            }
        } else if (num_bytes == 3) {
            code_point = (byte & 0x0F) << 12;
            code_point |= (data[i + 1] & 0x3F) << 6;
            code_point |= (data[i + 2] & 0x3F);
            if (code_point < 0x800) {
                return false; // Overlong encoding
            }
            // Check for surrogate pairs
            if (code_point >= 0xD800 && code_point <= 0xDFFF) {
                return false;
            }
        } else if (num_bytes == 4) {
            code_point = (byte & 0x07) << 18;
            code_point |= (data[i + 1] & 0x3F) << 12;
            code_point |= (data[i + 2] & 0x3F) << 6;
            code_point |= (data[i + 3] & 0x3F);
            if (code_point < 0x10000) {
                return false; // Overlong encoding
            }
            if (code_point > 0x10FFFF) {
                return false; // Beyond Unicode range
            }
        }
        
        i += num_bytes;
    }
    
    return true;
}

bool JsonString::is_valid_utf8(std::string_view input) noexcept {
    return validate_utf8(input);
}

std::string JsonString::escape(std::string_view input) {
    std::ostringstream escaped;
    escaped << "\"";
    
    for (char c : input) {
        switch (c) {
            case '"': escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b"; break;
            case '\f': escaped << "\\f"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default:
                if (static_cast<uint8_t>(c) < 0x20) {
                    // Control character - escape as \uXXXX
                    escaped << "\\u"
                           << std::hex << std::setw(4) << std::setfill('0')
                           << static_cast<int>(c);
                } else {
                    escaped << c;
                }
        }
    }
    
    escaped << "\"";
    return escaped.str();
}

std::string JsonString::unescape(std::string_view input) {
    if (input.empty() || input.front() != '"' || input.back() != '"') {
        throw JsonException("Invalid JSON string format");
    }
    
    std::string result;
    result.reserve(input.length() - 2);
    
    size_t i = 1; // Skip opening quote
    while (i < input.length() - 1) {
        char c = input[i++];
        
        if (c == '\\') {
            if (i >= input.length() - 1) {
                throw JsonException("Invalid escape sequence");
            }
            
            char next = input[i++];
            switch (next) {
                case '"': result += '"'; break;
                case '\\': result += '\\'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                case 'u': {
                    // Unicode escape
                    if (i + 4 > input.length() - 1) {
                        throw JsonException("Invalid Unicode escape sequence");
                    }
                    
                    std::string hex_str(input.substr(i, 4));
                    try {
                        int code_point = std::stoi(hex_str, nullptr, 16);
                        
                        // Convert code point to UTF-8
                        if (code_point <= 0x7F) {
                            result += static_cast<char>(code_point);
                        } else if (code_point <= 0x7FF) {
                            result += static_cast<char>(0xC0 | ((code_point >> 6) & 0x1F));
                            result += static_cast<char>(0x80 | (code_point & 0x3F));
                        } else if (code_point <= 0xFFFF) {
                            result += static_cast<char>(0xE0 | ((code_point >> 12) & 0x0F));
                            result += static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (code_point & 0x3F));
                        } else if (code_point <= 0x10FFFF) {
                            result += static_cast<char>(0xF0 | ((code_point >> 18) & 0x07));
                            result += static_cast<char>(0x80 | ((code_point >> 12) & 0x3F));
                            result += static_cast<char>(0x80 | ((code_point >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (code_point & 0x3F));
                        }
                    } catch (...) {
                        throw JsonException("Invalid Unicode escape sequence");
                    }
                    
                    i += 4;
                    break;
                }
                default:
                    throw JsonException("Invalid escape sequence");
            }
        } else {
            result += c;
        }
    }
    
    return result;
}

} // namespace jansson
