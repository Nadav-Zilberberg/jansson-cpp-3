#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_large_string..." << std::endl;
    
    // Create a large JSON string (more than 1KB)
    std::string large_json_str = "{\"data\": \"";
    for (int i = 0; i < 1000; i++) {
        large_json_str += "This is a test string segment ";
    }
    large_json_str += "\"}";
    
    std::cout << "Large JSON string size: " << large_json_str.size() << " bytes" << std::endl;
    
    // Parse the large JSON string
    json_error_t error;
    json_t* json = json_loads(large_json_str.c_str(), 0, &error);
    assert(json != nullptr);
    assert(json_is_object(json) == 1);
    
    // Verify the data field exists
    json_t* data = json_object_get(json, "data");
    assert(data != nullptr);
    assert(json_is_string(data) == 1);
    
    // Verify the string length
    const char* data_str = json_string_value(data);
    assert(data_str != nullptr);
    size_t data_len = strlen(data_str);
    std::cout << "Parsed string length: " << data_len << " bytes" << std::endl;
    assert(data_len > 1000);
    
    // Test serialization of large object
    char* serialized = json_dumps(json, 0);
    assert(serialized != nullptr);
    std::cout << "Serialized large JSON successfully" << std::endl;
    
    json_dumps_free(serialized);
    json_delete(json);
    
    std::cout << "test_large_string passed!" << std::endl;
    return 0;
}
