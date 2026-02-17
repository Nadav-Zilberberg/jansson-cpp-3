#ifndef STRING_UTILS_HPP
#define STRING_UTILS_HPP

#include <string>
#include <string_view>
#include <vector>

namespace jansson {

class JsonString {
public:
    // Construct from string (validates UTF-8)
    explicit JsonString(std::string value);
    
    // Construct from string_view (validates UTF-8)
    explicit JsonString(std::string_view value);
    
    // Get the underlying string
    const std::string& str() const noexcept { return value_; }
    
    // Get as string_view
    std::string_view view() const noexcept { return value_; }
    
    // Get C-style string
    const char* c_str() const noexcept { return value_.c_str(); }
    
    // Check if string is empty
    bool empty() const noexcept { return value_.empty(); }
    
    // Get length
    size_t length() const noexcept { return value_.length(); }
    
    // Escape string for JSON
    static std::string escape(std::string_view input);
    
    // Unescape JSON string
    static std::string unescape(std::string_view input);
    
    // Validate UTF-8
    static bool is_valid_utf8(std::string_view input) noexcept;
    
    // Comparison operators
    bool operator==(const JsonString& other) const noexcept;
    bool operator!=(const JsonString& other) const noexcept;
    bool operator<(const JsonString& other) const noexcept;

private:
    std::string value_;
    
    // Helper function to validate UTF-8
    static bool validate_utf8(std::string_view input) noexcept;
};

} // namespace jansson

#endif // STRING_UTILS_HPP
