#include "json_c_api.hpp"
#include "json_value.hpp"
#include "json_parser.hpp"
#include "json_serializer.hpp"
#include "json_error.hpp"
#include "string_utils.hpp"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

// Opaque json_t structure
typedef struct json_t {
    std::shared_ptr<jansson::JsonValue> value;
} json_t;

// Memory management
void* json_malloc(size_t size) {
    return ::operator new(size);
}

void json_free(void* ptr) {
    ::operator delete(ptr);
}

// JSON value creation and destruction
json_t* json_null() {
    json_t* json = new json_t;
    json->value = jansson::JsonNull::create();
    return json;
}

json_t* json_boolean(int value) {
    json_t* json = new json_t;
    json->value = jansson::JsonBoolean::create(value != 0);
    return json;
}

json_t* json_number(double value) {
    json_t* json = new json_t;
    json->value = jansson::JsonNumber::create(value);
    return json;
}

json_t* json_string(const char* value) {
    if (!value) {
        return nullptr;
    }
    json_t* json = new json_t;
    json->value = jansson::JsonStringValue::create(value);
    return json;
}

json_t* json_array() {
    json_t* json = new json_t;
    json->value = jansson::JsonArray::create();
    return json;
}

json_t* json_object() {
    json_t* json = new json_t;
    json->value = jansson::JsonObject::create();
    return json;
}

void json_delete(json_t* json) {
    if (json) {
        delete json;
    }
}

// Type checking
json_type json_typeof(const json_t* json) {
    if (!json || !json->value) {
        return JSON_NULL;
    }
    
    switch (json->value->type()) {
        case jansson::JsonType::Null: return JSON_NULL;
        case jansson::JsonType::Boolean: return JSON_BOOLEAN;
        case jansson::JsonType::Number: return JSON_NUMBER;
        case jansson::JsonType::String: return JSON_STRING;
        case jansson::JsonType::Array: return JSON_ARRAY;
        case jansson::JsonType::Object: return JSON_OBJECT;
        default: return JSON_NULL;
    }
}

int json_is_null(const json_t* json) {
    return json && json->value && json->value->is_null();
}

int json_is_boolean(const json_t* json) {
    return json && json->value && json->value->is_boolean();
}

int json_is_number(const json_t* json) {
    return json && json->value && json->value->is_number();
}

int json_is_string(const json_t* json) {
    return json && json->value && json->value->is_string();
}

int json_is_array(const json_t* json) {
    return json && json->value && json->value->is_array();
}

int json_is_object(const json_t* json) {
    return json && json->value && json->value->is_object();
}

// Value access
int json_boolean_value(const json_t* json) {
    if (!json || !json->value || !json->value->is_boolean()) {
        return 0;
    }
    return json->value->boolean_value() ? 1 : 0;
}

double json_number_value(const json_t* json) {
    if (!json || !json->value || !json->value->is_number()) {
        return 0.0;
    }
    return json->value->number_value();
}

const char* json_string_value(const json_t* json) {
    if (!json || !json->value || !json->value->is_string()) {
        return nullptr;
    }
    return json->value->string_value().c_str();
}

size_t json_array_size(const json_t* json) {
    if (!json || !json->value || !json->value->is_array()) {
        return 0;
    }
    return static_cast<const jansson::JsonArray*>(json->value.get())->size();
}

json_t* json_array_get(const json_t* json, size_t index) {
    if (!json || !json->value || !json->value->is_array()) {
        return nullptr;
    }
    
    try {
        auto value = static_cast<const jansson::JsonArray*>(json->value.get())->at(index);
        json_t* result = new json_t;
        result->value = value;
        return result;
    } catch (...) {
        return nullptr;
    }
}

size_t json_object_size(const json_t* json) {
    if (!json || !json->value || !json->value->is_object()) {
        return 0;
    }
    return static_cast<const jansson::JsonObject*>(json->value.get())->size();
}

json_t* json_object_get(const json_t* json, const char* key) {
    if (!json || !json->value || !json->value->is_object() || !key) {
        return nullptr;
    }
    
    auto value = static_cast<const jansson::JsonObject*>(json->value.get())->get(key);
    if (!value) {
        return nullptr;
    }
    
    json_t* result = new json_t;
    result->value = value;
    return result;
}

// Array operations
int json_array_append(json_t* json, json_t* value) {
    if (!json || !json->value || !json->value->is_array() || !value || !value->value) {
        return JSON_ERROR_INVALID_ARGUMENT;
    }
    
    try {
        static_cast<jansson::JsonArray*>(json->value.get())->push_back(value->value);
        return JSON_ERROR_SUCCESS;
    } catch (...) {
        return JSON_ERROR_MEMORY_ALLOCATION_FAILED;
    }
}

// Object operations
int json_object_set(json_t* json, const char* key, json_t* value) {
    if (!json || !json->value || !json->value->is_object() || !key || !value || !value->value) {
        return JSON_ERROR_INVALID_ARGUMENT;
    }
    
    try {
        static_cast<jansson::JsonObject*>(json->value.get())->set(key, value->value);
        return JSON_ERROR_SUCCESS;
    } catch (...) {
        return JSON_ERROR_MEMORY_ALLOCATION_FAILED;
    }
}

int json_object_del(json_t* json, const char* key) {
    if (!json || !json->value || !json->value->is_object() || !key) {
        return JSON_ERROR_INVALID_ARGUMENT;
    }
    
    try {
        static_cast<jansson::JsonObject*>(json->value.get())->erase(key);
        return JSON_ERROR_SUCCESS;
    } catch (...) {
        return JSON_ERROR_UNKNOWN_ERROR;
    }
}

// Parsing
json_t* json_loads(const char* input, size_t flags, json_error_code* error) {
    if (!input) {
        if (error) {
            *error = JSON_ERROR_INVALID_ARGUMENT;
        }
        return nullptr;
    }
    
    try {
        auto result = jansson::JsonParser::parse(input);
        if (!result) {
            if (error) {
                *error = JSON_ERROR_PARSE_ERROR;
            }
            return nullptr;
        }
        
        json_t* json = new json_t;
        json->value = result.value();
        if (error) {
            *error = JSON_ERROR_SUCCESS;
        }
        return json;
    } catch (...) {
        if (error) {
            *error = JSON_ERROR_PARSE_ERROR;
        }
        return nullptr;
    }
}

// Serialization
char* json_dumps(const json_t* json, size_t flags) {
    if (!json || !json->value) {
        return nullptr;
    }
    
    try {
        std::string result = jansson::JsonSerializer::serialize(*json->value);
        char* buffer = static_cast<char*>(json_malloc(result.size() + 1));
        if (!buffer) {
            return nullptr;
        }
        std::copy(result.begin(), result.end(), buffer);
        buffer[result.size()] = '\0';
        return buffer;
    } catch (...) {
        return nullptr;
    }
}

void json_dumps_free(char* json) {
    if (json) {
        json_free(json);
    }
}

// Error handling
const char* json_error_text(json_error_code error) {
    switch (error) {
        case JSON_ERROR_SUCCESS: return "Success";
        case JSON_ERROR_MEMORY_ALLOCATION_FAILED: return "Memory allocation failed";
        case JSON_ERROR_INVALID_UTF8: return "Invalid UTF-8 sequence";
        case JSON_ERROR_SYNTAX_ERROR: return "JSON syntax error";
        case JSON_ERROR_INVALID_TYPE: return "Invalid type";
        case JSON_ERROR_KEY_NOT_FOUND: return "Key not found";
        case JSON_ERROR_INDEX_OUT_OF_BOUNDS: return "Index out of bounds";
        case JSON_ERROR_INVALID_ARGUMENT: return "Invalid argument";
        case JSON_ERROR_PARSE_ERROR: return "Parse error";
        case JSON_ERROR_SERIALIZATION_ERROR: return "Serialization error";
        case JSON_ERROR_NOT_IMPLEMENTED: return "Not implemented";
        case JSON_ERROR_UNKNOWN_ERROR: return "Unknown error";
        default: return "Unknown error";
    }
}
