#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_string..." << std::endl;
    
    // Test creating string
    json_t* str_json = json_string("Hello, World!");
    assert(str_json != nullptr);
    assert(json_typeof(str_json) == JSON_STRING);
    assert(json_is_string(str_json) == 1);
    
    const char* str_value = json_string_value(str_json);
    assert(str_value != nullptr);
    assert(std::string(str_value) == "Hello, World!");
    
    // Test empty string
    json_t* empty_str_json = json_string("");
    assert(empty_str_json != nullptr);
    str_value = json_string_value(empty_str_json);
    assert(str_value != nullptr);
    assert(std::string(str_value) == "");
    
    // Test string with special characters
    json_t* special_str_json = json_string("Line1\nLine2\tTab");
    assert(special_str_json != nullptr);
    
    // Test serialization
    char* str_serialized = json_dumps(str_json, 0);
    assert(str_serialized != nullptr);
    std::cout << "String serialized: " << str_serialized << std::endl;
    assert(std::string(str_serialized) == "\"Hello, World!\"");
    
    json_dumps_free(str_serialized);
    json_delete(str_json);
    json_delete(empty_str_json);
    json_delete(special_str_json);
    
    std::cout << "test_string passed!" << std::endl;
    return 0;
}
