#include "json_serializer.hpp"
#include "string_utils.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace jansson {

void JsonSerializer::serialize_value(std::ostream& os, const JsonValue& value, bool pretty_print, int indent, int current_indent) {
    switch (value.type()) {
        case JsonType::Null:
            os << "null";
            break;
        case JsonType::Boolean:
            os << (value.boolean_value() ? "true" : "false");
            break;
        case JsonType::Number: {
            double num = value.number_value();
            if (std::floor(num) == num) {
                os << static_cast<int64_t>(num);
            } else {
                // Use default formatting for floating point
                os << num;
            }
            break;
        }
        case JsonType::String:
            os << value.to_string();
            break;
        case JsonType::Array:
            serialize_array(os, static_cast<const JsonArray&>(value), pretty_print, indent, current_indent);
            break;
        case JsonType::Object:
            serialize_object(os, static_cast<const JsonObject&>(value), pretty_print, indent, current_indent);
            break;
    }
}

void JsonSerializer::serialize_array(std::ostream& os, const JsonArray& array, bool pretty_print, int indent, int current_indent) {
    os << "[";
    
    if (pretty_print) {
        os << "\n";
    }
    
    bool first = true;
    for (const auto& item : array) {
        if (!first) {
            os << ",";
            if (pretty_print) {
                os << "\n";
            } else {
                os << " ";
            }
        }
        
        if (pretty_print) {
            os << std::string(current_indent + indent, ' ');
        }
        
        serialize_value(os, *item, pretty_print, indent, current_indent + indent);
        first = false;
    }
    
    if (pretty_print && !array.empty()) {
        os << "\n";
        os << std::string(current_indent, ' ');
    }
    
    os << "]";
}

void JsonSerializer::serialize_object(std::ostream& os, const JsonObject& object, bool pretty_print, int indent, int current_indent) {
    os << "{";
    
    if (pretty_print) {
        os << "\n";
    }
    
    bool first = true;
    for (const auto& [key, value] : object) {
        if (!first) {
            os << ",";
            if (pretty_print) {
                os << "\n";
            } else {
                os << " ";
            }
        }
        
        if (pretty_print) {
            os << std::string(current_indent + indent, ' ');
        }
        
        os << JsonString::escape(key);
        if (pretty_print) {
            os << " : ";
        } else {
            os << ": ";
        }
        
        serialize_value(os, *value, pretty_print, indent, current_indent + indent);
        first = false;
    }
    
    if (pretty_print && !object.empty()) {
        os << "\n";
        os << std::string(current_indent, ' ');
    }
    
    os << "}";
}

std::string JsonSerializer::serialize(const JsonValue& value) {
    std::ostringstream oss;
    serialize(oss, value, false, 2, 0);
    return oss.str();
}

std::string JsonSerializer::serialize(const JsonValue& value, bool pretty_print, int indent) {
    std::ostringstream oss;
    serialize(oss, value, pretty_print, indent, 0);
    return oss.str();
}

void JsonSerializer::serialize(std::ostream& os, const JsonValue& value, bool pretty_print, int indent, int current_indent) {
    serialize_value(os, value, pretty_print, indent, current_indent);
}

} // namespace jansson
