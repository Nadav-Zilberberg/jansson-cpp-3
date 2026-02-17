#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_object_operations..." << std::endl;
    
    // Create object
    json_t* obj = json_object();
    assert(obj != nullptr);
    
    // Test setting and getting values
    json_t* name = json_string("John");
    json_t* age = json_integer(30);
    
    assert(json_object_set(obj, "name", name) == 0);
    assert(json_object_set(obj, "age", age) == 0);
    assert(json_object_size(obj) == 2);
    
    // Test getting values
    json_t* value = json_object_get(obj, "name");
    assert(value != nullptr);
    assert(std::string(json_string_value(value)) == "John");
    
    value = json_object_get(obj, "age");
    assert(value != nullptr);
    assert(json_integer_value(value) == 30);
    
    // Test updating values
    json_t* new_name = json_string("Jane");
    assert(json_object_set(obj, "name", new_name) == 0);
    assert(json_object_size(obj) == 2); // Size should remain the same
    
    value = json_object_get(obj, "name");
    assert(value != nullptr);
    assert(std::string(json_string_value(value)) == "Jane");
    
    // Test removing values
    assert(json_object_del(obj, "age") == 0);
    assert(json_object_size(obj) == 1);
    
    value = json_object_get(obj, "age");
    assert(value == nullptr);
    
    // Test clearing object
    json_object_clear(obj);
    assert(json_object_size(obj) == 0);
    
    // Test serialization
    char* obj_str = json_dumps(obj, 0);
    assert(obj_str != nullptr);
    std::cout << "Object after operations: " << obj_str << std::endl;
    assert(std::string(obj_str) == "{}");
    
    json_dumps_free(obj_str);
    json_delete(obj);
    
    std::cout << "test_object_operations passed!" << std::endl;
    return 0;
}
