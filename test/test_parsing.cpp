#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_parsing..." << std::endl;
    
    // Test parsing simple JSON
    json_error_code error;
    json_t* json = json_loads("{\"name\": \"John\", \"age\": 30, \"active\": true}", 0, &error);
    assert(json != nullptr);
    assert(json_is_object(json) == 1);
    
    // Verify parsed values
    json_t* name = json_object_get(json, "name");
    assert(name != nullptr);
    assert(json_is_string(name) == 1);
    assert(std::string(json_string_value(name)) == "John");
    
    json_t* age = json_object_get(json, "age");
    assert(age != nullptr);
    assert(json_is_number(age) == 1);
    assert(json_number_value(age) == 30);
    
    json_t* active = json_object_get(json, "active");
    assert(active != nullptr);
    assert(json_is_boolean(active) == 1);
    assert(json_boolean_value(active) == 1);
    
    json_delete(json);
    
    // Test parsing array
    json = json_loads("[1, 2, 3, 4, 5]", 0, &error);
    assert(json != nullptr);
    assert(json_is_array(json) == 1);
    assert(json_array_size(json) == 5);
    
    json_delete(json);
    
    std::cout << "test_parsing passed!" << std::endl;
    return 0;
}
