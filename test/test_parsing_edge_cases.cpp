#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_parsing_edge_cases..." << std::endl;
    
    json_error_code error;
    
    // Test parsing empty object
    json_t* json = json_loads("{}", 0, &error);
    assert(json != nullptr);
    assert(json_is_object(json) == 1);
    assert(json_object_size(json) == 0);
    json_delete(json);
    
    // Test parsing empty array
    json = json_loads("[]", 0, &error);
    assert(json != nullptr);
    assert(json_is_array(json) == 1);
    assert(json_array_size(json) == 0);
    json_delete(json);
    
    // Test parsing null
    json = json_loads("null", 0, &error);
    assert(json != nullptr);
    assert(json_is_null(json) == 1);
    json_delete(json);
    
    // Test parsing true
    json = json_loads("true", 0, &error);
    assert(json != nullptr);
    assert(json_is_boolean(json) == 1);
    assert(json_boolean_value(json) == 1);
    json_delete(json);
    
    // Test parsing false
    json = json_loads("false", 0, &error);
    assert(json != nullptr);
    assert(json_is_boolean(json) == 1);
    assert(json_boolean_value(json) == 0);
    json_delete(json);
    
    // Test parsing zero
    json = json_loads("0", 0, &error);
    assert(json != nullptr);
    assert(json_is_number(json) == 1);
    assert(json_number_value(json) == 0);
    json_delete(json);
    
    // Test parsing negative number
    json = json_loads("-42", 0, &error);
    assert(json != nullptr);
    assert(json_is_number(json) == 1);
    assert(json_number_value(json) == -42);
    json_delete(json);
    
    // Test parsing floating point
    json = json_loads("3.14159", 0, &error);
    assert(json != nullptr);
    assert(json_is_number(json) == 1);
    json_delete(json);
    
    // Test parsing empty string
    json = json_loads("\"\"", 0, &error);
    assert(json != nullptr);
    assert(json_is_string(json) == 1);
    assert(std::string(json_string_value(json)) == "");
    json_delete(json);
    
    std::cout << "test_parsing_edge_cases passed!" << std::endl;
    return 0;
}
