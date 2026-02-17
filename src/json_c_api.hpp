#ifndef JSON_C_API_HPP
#define JSON_C_API_HPP

#include <cstddef>
#include <cstdint>

// Forward declarations for C API compatibility
extern "C" {

// Opaque type for JSON values
typedef struct json_t json_t;

// JSON types
typedef enum {
    JSON_NULL,
    JSON_BOOLEAN,
    JSON_NUMBER,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT
} json_type;

// Error codes
typedef enum {
    JSON_ERROR_SUCCESS = 0,
    JSON_ERROR_MEMORY_ALLOCATION_FAILED,
    JSON_ERROR_INVALID_UTF8,
    JSON_ERROR_SYNTAX_ERROR,
    JSON_ERROR_INVALID_TYPE,
    JSON_ERROR_KEY_NOT_FOUND,
    JSON_ERROR_INDEX_OUT_OF_BOUNDS,
    JSON_ERROR_INVALID_ARGUMENT,
    JSON_ERROR_PARSE_ERROR,
    JSON_ERROR_SERIALIZATION_ERROR,
    JSON_ERROR_NOT_IMPLEMENTED,
    JSON_ERROR_UNKNOWN_ERROR
} json_error_code;

// Memory management
void* json_malloc(size_t size);
void json_free(void* ptr);

// JSON value creation and destruction
json_t* json_null();
json_t* json_boolean(int value);
json_t* json_number(double value);
json_t* json_string(const char* value);
json_t* json_array();
json_t* json_object();
void json_delete(json_t* json);

// Type checking
json_type json_typeof(const json_t* json);
int json_is_null(const json_t* json);
int json_is_boolean(const json_t* json);
int json_is_number(const json_t* json);
int json_is_string(const json_t* json);
int json_is_array(const json_t* json);
int json_is_object(const json_t* json);

// Value access
int json_boolean_value(const json_t* json);
double json_number_value(const json_t* json);
const char* json_string_value(const json_t* json);
size_t json_array_size(const json_t* json);
json_t* json_array_get(const json_t* json, size_t index);
size_t json_object_size(const json_t* json);
json_t* json_object_get(const json_t* json, const char* key);

// Array operations
int json_array_append(json_t* json, json_t* value);
int json_array_insert(json_t* json, json_t* value, size_t index);
int json_array_remove(json_t* json, size_t index);
int json_array_clear(json_t* json);

// Object operations
int json_object_set(json_t* json, const char* key, json_t* value);
int json_object_del(json_t* json, const char* key);
int json_object_clear(json_t* json);

// Parsing
json_t* json_loads(const char* input, size_t flags, json_error_code* error);

// Serialization
char* json_dumps(const json_t* json, size_t flags);
void json_dumps_free(char* json);

// Error handling
const char* json_error_text(json_error_code error);

} // extern "C"

#endif // JSON_C_API_HPP
