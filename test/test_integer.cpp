#include <iostream>
#include <cassert>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_integer..." << std::endl;
    
    // Test creating integer
    json_t* int_json = json_integer(42);
    assert(int_json != nullptr);
    assert(json_typeof(int_json) == JSON_INTEGER);
    assert(json_is_number(int_json) == 1);
    assert(json_integer_value(int_json) == 42);
    
    // Test negative integer
    json_t* neg_int_json = json_integer(-123);
    assert(neg_int_json != nullptr);
    assert(json_integer_value(neg_int_json) == -123);
    
    // Test zero
    json_t* zero_json = json_integer(0);
    assert(zero_json != nullptr);
    assert(json_integer_value(zero_json) == 0);
    
    // Test serialization
    char* int_str = json_dumps(int_json, 0);
    assert(int_str != nullptr);
    std::cout << "Integer serialized: " << int_str << std::endl;
    assert(std::string(int_str) == "42");
    
    json_dumps_free(int_str);
    json_delete(int_json);
    json_delete(neg_int_json);
    json_delete(zero_json);
    
    std::cout << "test_integer passed!" << std::endl;
    return 0;
}
