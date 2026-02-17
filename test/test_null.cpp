#include <iostream>
#include <cassert>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_null..." << std::endl;
    
    // Test creating null JSON
    json_t* null_json = json_null();
    assert(null_json != nullptr);
    assert(json_typeof(null_json) == JSON_NULL);
    assert(json_is_null(null_json) == 1);
    assert(json_is_boolean(null_json) == 0);
    assert(json_is_number(null_json) == 0);
    assert(json_is_string(null_json) == 0);
    assert(json_is_array(null_json) == 0);
    assert(json_is_object(null_json) == 0);
    
    // Test serialization
    char* json_str = json_dumps(null_json, 0);
    assert(json_str != nullptr);
    std::cout << "Serialized: " << json_str << std::endl;
    assert(std::string(json_str) == "null");
    
    json_dumps_free(json_str);
    json_delete(null_json);
    
    std::cout << "test_null passed!" << std::endl;
    return 0;
}
