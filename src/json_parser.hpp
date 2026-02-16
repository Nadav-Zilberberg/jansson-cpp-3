#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <string>
#include <string_view>
#include <memory>
#include "json_value.hpp"
#include "json_error.hpp"

namespace jansson {

class JsonParser {
public:
    // Parse JSON from string
    static Result<std::shared_ptr<JsonValue>> parse(std::string_view input);
    
    // Parse JSON from string with error reporting
    static Result<std::shared_ptr<JsonValue>> parse_with_error(
        std::string_view input,
        std::string& error_message,
        size_t& error_position
    );

private:
    struct ParseContext {
        std::string_view input;
        size_t position = 0;
        std::string error_message;
        size_t error_position = 0;
    };
    
    static std::shared_ptr<JsonValue> parse_value(ParseContext& ctx);
    static std::shared_ptr<JsonObject> parse_object(ParseContext& ctx);
    static std::shared_ptr<JsonArray> parse_array(ParseContext& ctx);
    static std::shared_ptr<JsonStringValue> parse_string(ParseContext& ctx);
    static std::shared_ptr<JsonNumber> parse_number(ParseContext& ctx);
    static std::shared_ptr<JsonBoolean> parse_boolean(ParseContext& ctx);
    static std::shared_ptr<JsonNull> parse_null(ParseContext& ctx);
    
    static void skip_whitespace(ParseContext& ctx);
    static char peek(ParseContext& ctx);
    static char consume(ParseContext& ctx);
    static void expect(ParseContext& ctx, char expected);
    static std::string parse_raw_string(ParseContext& ctx);
};

} // namespace jansson

#endif // JSON_PARSER_HPP
