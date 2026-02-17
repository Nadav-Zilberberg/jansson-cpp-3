#include <iostream>
#include <cassert>
#include <string>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_complex_nested..." << std::endl;
    
    // Create a complex nested structure
    json_t* root = json_object();
    
    // Add simple values
    json_object_set(root, "name", json_string("Root Object"));
    json_object_set(root, "version", json_number(1));
    json_object_set(root, "active", json_boolean(1));
    
    // Add array of objects
    json_t* items_array = json_array();
    for (int i = 0; i < 5; i++) {
        json_t* item = json_object();
        char key[20];
        sprintf(key, "item_%d", i);
        json_object_set(item, "id", json_number(i));
        json_object_set(item, "name", json_string(key));
        json_object_set(item, "value", json_number(i * 1.5));
        json_array_append(items_array, item);
    }
    json_object_set(root, "items", items_array);
    
    // Add nested object with array
    json_t* config = json_object();
    json_object_set(config, "enabled", json_boolean(1));
    json_object_set(config, "timeout", json_number(30));
    
    json_t* tags = json_array();
    json_array_append(tags, json_string("tag1"));
    json_array_append(tags, json_string("tag2"));
    json_array_append(tags, json_string("tag3"));
    json_object_set(config, "tags", tags);
    
    json_object_set(root, "config", config);
    
    // Verify structure
    assert(json_object_size(root) == 5);
    
    json_t* items = json_object_get(root, "items");
    assert(json_is_array(items) == 1);
    assert(json_array_size(items) == 5);
    
    json_t* first_item = json_array_get(items, 0);
    assert(json_is_object(first_item) == 1);
    assert(json_object_size(first_item) == 3);
    
    json_t* config_obj = json_object_get(root, "config");
    assert(json_is_object(config_obj) == 1);
    
    json_t* config_tags = json_object_get(config_obj, "tags");
    assert(json_is_array(config_tags) == 1);
    assert(json_array_size(config_tags) == 3);
    
    // Test serialization
    char* root_str = json_dumps(root, 0);
    assert(root_str != nullptr);
    std::cout << "Complex nested structure serialized successfully" << std::endl;
    
    json_dumps_free(root_str);
    json_delete(root);
    
    std::cout << "test_complex_nested passed!" << std::endl;
    return 0;
}
