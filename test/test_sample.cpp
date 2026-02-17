#include <iostream>
#include <cassert>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_sample..." << std::endl;
    
    // Test basic JSON creation
    json_t* obj = json_object();
    assert(obj != nullptr);
    
    json_t* name = json_string("John");
    assert(name != nullptr);
    
    int result = json_object_set(obj, "name", name);
    assert(result == 0);
    
    char* json_str = json_dumps(obj, 0);
    assert(json_str != nullptr);
    
    std::cout << "JSON: " << json_str << std::endl;
    
    json_dumps_free(json_str);
    json_delete(obj);
    
    std::cout << "test_sample passed!" << std::endl;
    return 0;
}
