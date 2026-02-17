#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_mixed_types..." << std::endl;
    
    // Create an object with mixed types
    json_t* obj = json_object();
    
    // Add null
    json_t* null_val = json_null();
    assert(json_object_set(obj, "null_value", null_val) == 0);
    
    // Add boolean
    json_t* bool_val = json_boolean(1);
    assert(json_object_set(obj, "bool_value", bool_val) == 0);
    
    // Add integer
    json_t* int_val = json_integer(42);
    assert(json_object_set(obj, "int_value", int_val) == 0);
    
    // Add real
    json_t* real_val = json_real(3.14159);
    assert(json_object_set(obj, "real_value", real_val) == 0);
    
    // Add string
    json_t* str_val = json_string("test string");
    assert(json_object_set(obj, "str_value", str_val) == 0);
    
    // Add array
    json_t* arr_val = json_array();
    json_t* arr_elem1 = json_integer(1);
    json_t* arr_elem2 = json_integer(2);
    assert(json_array_append(arr_val, arr_elem1) == 0);
    assert(json_array_append(arr_val, arr_elem2) == 0);
    assert(json_object_set(obj, "array_value", arr_val) == 0);
    
    // Add nested object
    json_t* nested_obj = json_object();
    json_t* nested_key = json_string("nested_key");
    json_t* nested_val = json_string("nested_value");
    assert(json_object_set(nested_obj, "nested_key", nested_key) == 0);
    assert(json_object_set(nested_obj, "nested_value", nested_val) == 0);
    assert(json_object_set(obj, "nested_object", nested_obj) == 0);
    
    // Verify all types
    assert(json_object_size(obj) == 7);
    
    json_t* value = json_object_get(obj, "null_value");
    assert(json_is_null(value) == 1);
    
    value = json_object_get(obj, "bool_value");
    assert(json_is_boolean(value) == 1);
    assert(json_boolean_value(value) == 1);
    
    value = json_object_get(obj, "int_value");
    assert(json_is_integer(value) == 1);
    assert(json_integer_value(value) == 42);
    
    value = json_object_get(obj, "real_value");
    assert(json_is_real(value) == 1);
    
    value = json_object_get(obj, "str_value");
    assert(json_is_string(value) == 1);
    assert(std::string(json_string_value(value)) == "test string");
    
    value = json_object_get(obj, "array_value");
    assert(json_is_array(value) == 1);
    assert(json_array_size(value) == 2);
    
    value = json_object_get(obj, "nested_object");
    assert(json_is_object(value) == 1);
    assert(json_object_size(value) == 2);
    
    // Test serialization
    char* obj_str = json_dumps(obj, 0);
    assert(obj_str != nullptr);
    std::cout << "Mixed types object: " << obj_str << std::endl;
    
    json_dumps_free(obj_str);
    json_delete(obj);
    
    std::cout << "test_mixed_types passed!" << std::endl;
    return 0;
}
