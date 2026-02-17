#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_array..." << std::endl;
    
    // Test creating array
    json_t* arr = json_array();
    assert(arr != nullptr);
    assert(json_typeof(arr) == JSON_ARRAY);
    assert(json_is_array(arr) == 1);
    assert(json_array_size(arr) == 0);
    
    // Test adding elements
    json_t* int1 = json_integer(1);
    json_t* int2 = json_integer(2);
    json_t* int3 = json_integer(3);
    
    assert(json_array_append(arr, int1) == 0);
    assert(json_array_size(arr) == 1);
    assert(json_array_append(arr, int2) == 0);
    assert(json_array_size(arr) == 2);
    assert(json_array_append(arr, int3) == 0);
    assert(json_array_size(arr) == 3);
    
    // Test getting elements
    json_t* elem = json_array_get(arr, 0);
    assert(elem != nullptr);
    assert(json_integer_value(elem) == 1);
    
    elem = json_array_get(arr, 1);
    assert(elem != nullptr);
    assert(json_integer_value(elem) == 2);
    
    elem = json_array_get(arr, 2);
    assert(elem != nullptr);
    assert(json_integer_value(elem) == 3);
    
    // Test serialization
    char* arr_str = json_dumps(arr, 0);
    assert(arr_str != nullptr);
    std::cout << "Array serialized: " << arr_str << std::endl;
    
    json_dumps_free(arr_str);
    json_delete(arr);
    
    std::cout << "test_array passed!" << std::endl;
    return 0;
}
