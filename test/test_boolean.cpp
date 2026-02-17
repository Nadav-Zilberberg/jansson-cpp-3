#include <iostream>
#include <cassert>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_boolean..." << std::endl;
    
    // Test creating true boolean
    json_t* true_json = json_boolean(1);
    assert(true_json != nullptr);
    assert(json_typeof(true_json) == JSON_BOOLEAN);
    assert(json_is_boolean(true_json) == 1);
    assert(json_boolean_value(true_json) == 1);
    
    // Test creating false boolean
    json_t* false_json = json_boolean(0);
    assert(false_json != nullptr);
    assert(json_typeof(false_json) == JSON_BOOLEAN);
    assert(json_is_boolean(false_json) == 1);
    assert(json_boolean_value(false_json) == 0);
    
    // Test serialization
    char* true_str = json_dumps(true_json, 0);
    assert(true_str != nullptr);
    std::cout << "True serialized: " << true_str << std::endl;
    assert(std::string(true_str) == "true");
    
    char* false_str = json_dumps(false_json, 0);
    assert(false_str != nullptr);
    std::cout << "False serialized: " << false_str << std::endl;
    assert(std::string(false_str) == "false");
    
    json_dumps_free(true_str);
    json_dumps_free(false_str);
    json_delete(true_json);
    json_delete(false_json);
    
    std::cout << "test_boolean passed!" << std::endl;
    return 0;
}
