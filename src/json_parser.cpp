#include "json_parser.hpp"
#include "string_utils.hpp"
#include <cctype>
#include <sstream>
#include <charconv>
#include <cmath>

namespace jansson {

void JsonParser::skip_whitespace(ParseContext& ctx) {
    while (ctx.position < ctx.input.length()) {
        char c = ctx.input[ctx.position];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            ctx.position++;
        } else {
            break;
        }
    }
}

char JsonParser::peek(ParseContext& ctx) {
    skip_whitespace(ctx);
    if (ctx.position >= ctx.input.length()) {
        return '\0';
    }
    return ctx.input[ctx.position];
}

char JsonParser::consume(ParseContext& ctx) {
    skip_whitespace(ctx);
    if (ctx.position >= ctx.input.length()) {
        return '\0';
    }
    return ctx.input[ctx.position++];
}

void JsonParser::expect(ParseContext& ctx, char expected) {
    char c = consume(ctx);
    if (c != expected) {
        ctx.error_message = "Expected '";
        ctx.error_message += expected;
        ctx.error_message += "' but found '";
        ctx.error_message += c;
        ctx.error_message += "';";
        ctx.error_position = ctx.position - 1;
        throw JsonException(ctx.error_message);
    }
}

std::string JsonParser::parse_raw_string(ParseContext& ctx) {
    std::string result;
    
    expect(ctx, '"');
    
    while (true) {
        char c = consume(ctx);
        
        if (c == '"') {
            break;
        }
        
        if (c == '\\') {
            c = consume(ctx);
            switch (c) {
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
                    std::string hex_str;
                    for (int i = 0; i < 4; ++i) {
                        hex_str += consume(ctx);
                    }
                    
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
                        ctx.error_message = "Invalid Unicode escape sequence";
                        ctx.error_position = ctx.position - 5;
                        throw JsonException(ctx.error_message);
                    }
                    break;
                }
                default:
                    ctx.error_message = "Invalid escape sequence";
                    ctx.error_position = ctx.position - 1;
                    throw JsonException(ctx.error_message);
            }
        } else {
            result += c;
        }
    }
    
    return result;
}

std::shared_ptr<JsonStringValue> JsonParser::parse_string(ParseContext& ctx) {
    std::string raw_str = parse_raw_string(ctx);
    return JsonStringValue::create(raw_str);
}

std::shared_ptr<JsonNumber> JsonParser::parse_number(ParseContext& ctx) {
    std::string num_str;
    char c = peek(ctx);
    
    // Handle optional sign
    if (c == '-' || c == '+') {
        num_str += consume(ctx);
        c = peek(ctx);
    }
    
    // Parse integer part
    if (c == '0') {
        num_str += consume(ctx);
        c = peek(ctx);
        // Check for hex, octal, or binary (not standard JSON but some implementations support)
        // For standard JSON, we just accept single 0
    } else if (std::isdigit(c)) {
        while (std::isdigit(peek(ctx))) {
            num_str += consume(ctx);
        }
    } else {
        ctx.error_message = "Invalid number format";
        ctx.error_position = ctx.position;
        throw JsonException(ctx.error_message);
    }
    
    c = peek(ctx);
    
    // Parse fractional part
    if (c == '.') {
        num_str += consume(ctx);
        if (!std::isdigit(peek(ctx))) {
            ctx.error_message = "Invalid number format - expected digit after decimal point";
            ctx.error_position = ctx.position;
            throw JsonException(ctx.error_message);
        }
        while (std::isdigit(peek(ctx))) {
            num_str += consume(ctx);
        }
        c = peek(ctx);
    }
    
    // Parse exponent
    if (c == 'e' || c == 'E') {
        num_str += consume(ctx);
        c = peek(ctx);
        if (c == '+' || c == '-') {
            num_str += consume(ctx);
        }
        if (!std::isdigit(peek(ctx))) {
            ctx.error_message = "Invalid number format - expected digit in exponent";
            ctx.error_position = ctx.position;
            throw JsonException(ctx.error_message);
        }
        while (std::isdigit(peek(ctx))) {
            num_str += consume(ctx);
        }
    }
    
    try {
        double value = std::stod(num_str);
        return JsonNumber::create(value);
    } catch (...) {
        ctx.error_message = "Invalid number format";
        ctx.error_position = ctx.position - num_str.length();
        throw JsonException(ctx.error_message);
    }
}

std::shared_ptr<JsonBoolean> JsonParser::parse_boolean(ParseContext& ctx) {
    if (peek(ctx) == 't') {
        expect(ctx, 't');
        expect(ctx, 'r');
        expect(ctx, 'u');
        expect(ctx, 'e');
        return JsonBoolean::create(true);
    } else if (peek(ctx) == 'f') {
        expect(ctx, 'f');
        expect(ctx, 'a');
        expect(ctx, 'l');
        expect(ctx, 's');
        expect(ctx, 'e');
        return JsonBoolean::create(false);
    }
    
    ctx.error_message = "Invalid boolean value";
    ctx.error_position = ctx.position;
    throw JsonException(ctx.error_message);
}

std::shared_ptr<JsonNull> JsonParser::parse_null(ParseContext& ctx) {
    expect(ctx, 'n');
    expect(ctx, 'u');
    expect(ctx, 'l');
    expect(ctx, 'l');
    return JsonNull::create();
}

std::shared_ptr<JsonArray> JsonParser::parse_array(ParseContext& ctx) {
    auto array = JsonArray::create();
    
    expect(ctx, '[');
    skip_whitespace(ctx);
    
    if (peek(ctx) != ']') {
        while (true) {
            array->push_back(parse_value(ctx));
            skip_whitespace(ctx);
            
            char c = peek(ctx);
            if (c == ']') {
                break;
            } else if (c == ',') {
                consume(ctx);
                skip_whitespace(ctx);
            } else {
                ctx.error_message = "Expected ',' or ']' in array";
                ctx.error_position = ctx.position;
                throw JsonException(ctx.error_message);
            }
        }
    }
    
    expect(ctx, ']');
    return array;
}

std::shared_ptr<JsonObject> JsonParser::parse_object(ParseContext& ctx) {
    auto object = JsonObject::create();
    
    expect(ctx, '{');
    skip_whitespace(ctx);
    
    if (peek(ctx) != '}') {
        while (true) {
            std::string key = parse_raw_string(ctx);
            skip_whitespace(ctx);
            expect(ctx, ':');
            skip_whitespace(ctx);
            
            object->set(key, parse_value(ctx));
            skip_whitespace(ctx);
            
            char c = peek(ctx);
            if (c == '}') {
                break;
            } else if (c == ',') {
                consume(ctx);
                skip_whitespace(ctx);
            } else {
                ctx.error_message = "Expected ',' or '}' in object";
                ctx.error_position = ctx.position;
                throw JsonException(ctx.error_message);
            }
        }
    }
    
    expect(ctx, '}');
    return object;
}

std::shared_ptr<JsonValue> JsonParser::parse_value(ParseContext& ctx) {
    skip_whitespace(ctx);
    
    if (ctx.position >= ctx.input.length()) {
        ctx.error_message = "Unexpected end of input";
        ctx.error_position = ctx.position;
        throw JsonException(ctx.error_message);
    }
    
    char c = peek(ctx);
    
    switch (c) {
        case '"':
            return parse_string(ctx);
        case '{':
            return parse_object(ctx);
        case '[':
            return parse_array(ctx);
        case 't':
        case 'f':
            return parse_boolean(ctx);
        case 'n':
            return parse_null(ctx);
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return parse_number(ctx);
        default:
            ctx.error_message = "Unexpected character";
            ctx.error_position = ctx.position;
            throw JsonException(ctx.error_message);
    }
}

Result<std::shared_ptr<JsonValue>> JsonParser::parse(std::string_view input) {
    try {
        ParseContext ctx;
        ctx.input = input;
        ctx.position = 0;
        
        auto result = parse_value(ctx);
        skip_whitespace(ctx);
        
        if (ctx.position < ctx.input.length()) {
            std::string error_msg = "Unexpected trailing characters at position ";
            error_msg += std::to_string(ctx.position);
            return Result<std::shared_ptr<JsonValue>>(make_error_code(JsonErrorCode::SyntaxError));
        }
        
        return Result<std::shared_ptr<JsonValue>>(result);
    } catch (const JsonException& e) {
        return Result<std::shared_ptr<JsonValue>>(make_error_code(JsonErrorCode::ParseError));
    } catch (...) {
        return Result<std::shared_ptr<JsonValue>>(make_error_code(JsonErrorCode::UnknownError));
    }
}

Result<std::shared_ptr<JsonValue>> JsonParser::parse_with_error(
    std::string_view input,
    std::string& error_message,
    size_t& error_position
) {
    try {
        ParseContext ctx;
        ctx.input = input;
        ctx.position = 0;
        
        auto result = parse_value(ctx);
        skip_whitespace(ctx);
        
        if (ctx.position < ctx.input.length()) {
            error_message = "Unexpected trailing characters at position ";
            error_message += std::to_string(ctx.position);
            error_position = ctx.position;
            return Result<std::shared_ptr<JsonValue>>(make_error_code(JsonErrorCode::SyntaxError));
        }
        
        return Result<std::shared_ptr<JsonValue>>(result);
    } catch (const JsonException& e) {
        error_message = e.what();
        error_position = 0; // Would be set in context if we had it
        return Result<std::shared_ptr<JsonValue>>(make_error_code(JsonErrorCode::ParseError));
    } catch (...) {
        error_message = "Unknown error during parsing";
        error_position = 0;
        return Result<std::shared_ptr<JsonValue>>(make_error_code(JsonErrorCode::UnknownError));
    }
}

} // namespace jansson
