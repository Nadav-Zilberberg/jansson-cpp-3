#include <iostream>
#include <cassert>
#include <string>
#include <cstring>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_large_array..." << std::endl;
    
    // Create a large array with 1000 elements
    json_t* arr = json_array();
    assert(arr != nullptr);
    
    for (int i = 0; i < 1000; i++) {
        json_t* elem = json_number(i);
        assert(json_array_append(arr, elem) == 0);
    }
    
    assert(json_array_size(arr) == 1000);
    
    // Verify some elements
    json_t* elem = json_array_get(arr, 0);
    assert(json_number_value(elem) == 0);
    
    elem = json_array_get(arr, 500);
    assert(json_number_value(elem) == 500);
    
    elem = json_array_get(arr, 999);
    assert(json_number_value(elem) == 999);
    
    // Test serialization of large array
    char* arr_str = json_dumps(arr, 0);
    assert(arr_str != nullptr);
    std::cout << "Large array serialized successfully" << std::endl;
    std::cout << "Serialized size: " << strlen(arr_str) << " bytes" << std::endl;
    
    json_dumps_free(arr_str);
    json_delete(arr);
    
    std::cout << "test_large_array passed!" << std::endl;
    return 0;
}
