#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_nested..." << std::endl;
    
    // Test creating nested objects
    json_t* inner_obj = json_object();
    json_t* outer_obj = json_object();
    
    json_t* inner_name = json_string("Inner Name");
    json_t* inner_value = json_integer(123);
    
    assert(json_object_set(inner_obj, "name", inner_name) == 0);
    assert(json_object_set(inner_obj, "value", inner_value) == 0);
    
    assert(json_object_set(outer_obj, "inner", inner_obj) == 0);
    
    // Test creating nested arrays
    json_t* inner_array = json_array();
    json_t* outer_array = json_array();
    
    json_t* arr_elem1 = json_integer(1);
    json_t* arr_elem2 = json_integer(2);
    
    assert(json_array_append(inner_array, arr_elem1) == 0);
    assert(json_array_append(inner_array, arr_elem2) == 0);
    
    assert(json_array_append(outer_array, inner_array) == 0);
    
    // Test serialization
    char* nested_obj_str = json_dumps(outer_obj, 0);
    assert(nested_obj_str != nullptr);
    std::cout << "Nested object: " << nested_obj_str << std::endl;
    
    char* nested_arr_str = json_dumps(outer_array, 0);
    assert(nested_arr_str != nullptr);
    std::cout << "Nested array: " << nested_arr_str << std::endl;
    
    json_dumps_free(nested_obj_str);
    json_dumps_free(nested_arr_str);
    json_delete(outer_obj);
    json_delete(outer_array);
    
    std::cout << "test_nested passed!" << std::endl;
    return 0;
}
