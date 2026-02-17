#include <iostream>
#include <cassert>
#include <string>
#include <cstring>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_parsing_large_json..." << std::endl;
    
    // Create a large JSON string with nested structures
    std::string large_json = "{\"users\": [";
    
    for (int i = 0; i < 100; i++) {
        if (i > 0) large_json += ",";
        char user[200];
        sprintf(user, "{\"id\": %d, \"name\": \"User %d\", \"email\": \"user%d@example.com\", \"active\": %s, \"score\": %.2f}", 
                i, i, i, (i % 2 == 0) ? "true" : "false", i * 1.5);
        large_json += user;
    }
    
    large_json += "], \"metadata\": {\"count\": 100, \"timestamp\": 1234567890, \"version\": \"1.0\"}}";
    
    std::cout << "Large JSON string size: " << large_json.size() << " bytes" << std::endl;
    
    // Parse the large JSON
    json_error_code error;
    json_t* json = json_loads(large_json.c_str(), 0, &error);
    assert(json != nullptr);
    assert(json_is_object(json) == 1);
    
    // Verify structure
    json_t* users = json_object_get(json, "users");
    assert(users != nullptr);
    assert(json_is_array(users) == 1);
    assert(json_array_size(users) == 100);
    
    // Verify first user
    json_t* first_user = json_array_get(users, 0);
    assert(first_user != nullptr);
    assert(json_is_object(first_user) == 1);
    
    json_t* user_id = json_object_get(first_user, "id");
    assert(user_id != nullptr);
    assert(json_number_value(user_id) == 0);
    
    json_t* user_name = json_object_get(first_user, "name");
    assert(user_name != nullptr);
    std::cout << "First user name: " << json_string_value(user_name) << std::endl;
    assert(std::string(json_string_value(user_name)) == "User0");
    
    // Verify last user
    json_t* last_user = json_array_get(users, 99);
    assert(last_user != nullptr);
    
    user_id = json_object_get(last_user, "id");
    assert(user_id != nullptr);
    assert(json_number_value(user_id) == 99);
    
    // Verify metadata
    json_t* metadata = json_object_get(json, "metadata");
    assert(metadata != nullptr);
    assert(json_is_object(metadata) == 1);
    
    json_t* count = json_object_get(metadata, "count");
    assert(count != nullptr);
    assert(json_number_value(count) == 100);
    
    // Test serialization
    char* serialized = json_dumps(json, 0);
    assert(serialized != nullptr);
    std::cout << "Large JSON parsed and serialized successfully" << std::endl;
    std::cout << "Serialized size: " << strlen(serialized) << " bytes" << std::endl;
    
    json_dumps_free(serialized);
    json_delete(json);
    
    std::cout << "test_parsing_large_json passed!" << std::endl;
    return 0;
}
