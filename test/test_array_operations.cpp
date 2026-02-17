#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_array_operations..." << std::endl;
    
    // Create array
    json_t* arr = json_array();
    assert(arr != nullptr);
    
    // Test inserting at specific positions
    json_t* val1 = json_number(10);
    json_t* val2 = json_number(20);
    json_t* val3 = json_number(30);
    
    assert(json_array_append(arr, val1) == 0);
    assert(json_array_size(arr) == 1);
    
    assert(json_array_insert(arr, val2, 0) == 0);
    assert(json_array_size(arr) == 2);
    
    assert(json_array_insert(arr, val3, 1) == 0);
    assert(json_array_size(arr) == 3);
    
    // Verify order
    json_t* elem = json_array_get(arr, 0);
    assert(json_number_value(elem) == 20);
    
    elem = json_array_get(arr, 1);
    assert(json_number_value(elem) == 30);
    
    elem = json_array_get(arr, 2);
    assert(json_number_value(elem) == 10);
    
    // Test removing elements
    assert(json_array_remove(arr, 1) == 0);
    assert(json_array_size(arr) == 2);
    
    elem = json_array_get(arr, 0);
    assert(json_number_value(elem) == 20);
    
    elem = json_array_get(arr, 1);
    assert(json_number_value(elem) == 10);
    
    // Test clearing array
    json_array_clear(arr);
    assert(json_array_size(arr) == 0);
    
    // Test serialization
    char* arr_str = json_dumps(arr, 0);
    assert(arr_str != nullptr);
    std::cout << "Array after operations: " << arr_str << std::endl;
    assert(std::string(arr_str) == "[]");
    
    json_dumps_free(arr_str);
    json_delete(arr);
    
    std::cout << "test_array_operations passed!" << std::endl;
    return 0;
}
