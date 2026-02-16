#ifndef JSON_SERIALIZER_HPP
#define JSON_SERIALIZER_HPP

#include <string>
#include <memory>
#include "json_value.hpp"

namespace jansson {

class JsonSerializer {
public:
    // Serialize JSON value to string
    static std::string serialize(const JsonValue& value);
    
    // Serialize with formatting options
    static std::string serialize(const JsonValue& value, bool pretty_print, int indent = 2);
    
    // Serialize to stream
    static void serialize(std::ostream& os, const JsonValue& value, bool pretty_print = false, int indent = 2, int current_indent = 0);

private:
    static void serialize_value(std::ostream& os, const JsonValue& value, bool pretty_print, int indent, int current_indent);
    static void serialize_object(std::ostream& os, const JsonObject& object, bool pretty_print, int indent, int current_indent);
    static void serialize_array(std::ostream& os, const JsonArray& array, bool pretty_print, int indent, int current_indent);
};

} // namespace jansson

#endif // JSON_SERIALIZER_HPP
