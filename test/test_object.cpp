#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_object..." << std::endl;
    
    // Test creating object
    json_t* obj = json_object();
    assert(obj != nullptr);
    assert(json_typeof(obj) == JSON_OBJECT);
    assert(json_is_object(obj) == 1);
    assert(json_object_size(obj) == 0);
    
    // Test adding key-value pairs
    json_t* name = json_string("John");
    json_t* age = json_number(30);
    json_t* is_active = json_boolean(1);
    
    assert(json_object_set(obj, "name", name) == 0);
    assert(json_object_size(obj) == 1);
    assert(json_object_set(obj, "age", age) == 0);
    assert(json_object_size(obj) == 2);
    assert(json_object_set(obj, "is_active", is_active) == 0);
    assert(json_object_size(obj) == 3);
    
    // Test getting values
    json_t* value = json_object_get(obj, "name");
    assert(value != nullptr);
    assert(json_is_string(value) == 1);
    assert(std::string(json_string_value(value)) == "John");
    
    value = json_object_get(obj, "age");
    assert(value != nullptr);
    assert(json_is_number(value) == 1);
    assert(json_number_value(value) == 30);
    
    value = json_object_get(obj, "is_active");
    assert(value != nullptr);
    assert(json_is_boolean(value) == 1);
    assert(json_boolean_value(value) == 1);
    
    // Test serialization
    char* obj_str = json_dumps(obj, 0);
    assert(obj_str != nullptr);
    std::cout << "Object serialized: " << obj_str << std::endl;
    
    json_dumps_free(obj_str);
    json_delete(obj);
    
    std::cout << "test_object passed!" << std::endl;
    return 0;
}
