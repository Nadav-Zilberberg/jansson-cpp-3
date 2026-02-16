#include <iostream>
#include <string>
#include "json_value.hpp"
#include "json_parser.hpp"
#include "json_serializer.hpp"
#include "json_error.hpp"
#include "json_c_api.hpp"

int main() {
    std::cout << "=== Jansson C++ JSON Library Demo ===\n\n";
    
    // 1. Create JSON values using C++ API
    std::cout << "1. Creating JSON values using C++ API:\n";
    auto obj = jansson::JsonObject::create();
    obj->set("name", jansson::JsonStringValue::create("John Doe"));
    obj->set("age", jansson::JsonNumber::create(30));
    obj->set("is_active", jansson::JsonBoolean::create(true));
    obj->set("null_value", jansson::JsonNull::create());
    
    auto arr = jansson::JsonArray::create();
    arr->push_back(jansson::JsonNumber::create(1));
    arr->push_back(jansson::JsonNumber::create(2));
    arr->push_back(jansson::JsonNumber::create(3));
    obj->set("scores", arr);
    
    std::cout << "Created object: " << jansson::JsonSerializer::serialize(*obj, true) << "\n\n";
    
    // 2. Parse JSON string
    std::cout << "2. Parsing JSON string:\n";
    std::string json_str = R"({
        "title": "Book",
        "author": "Author Name",
        "year": 2023,
        "tags": ["fiction", "adventure"]
    })";
    
    auto parse_result = jansson::JsonParser::parse(json_str);
    if (parse_result) {
        std::cout << "Parsed successfully!\n";
        std::cout << "Parsed object: " << jansson::JsonSerializer::serialize(*parse_result.value(), true) << "\n\n";
    } else {
        std::cout << "Parse error: " << parse_result.error().message() << "\n\n";
    }
    
    // 3. Demonstrate C API compatibility
    std::cout << "3. Using C API:\n";
    json_t* c_obj = json_object();
    json_object_set(c_obj, "message", json_string("Hello from C API!"));
    json_object_set(c_obj, "version", json_number(1.0));
    
    char* c_json_str = json_dumps(c_obj, 0);
    if (c_json_str) {
        std::cout << "C API result: " << c_json_str << "\n";
        json_dumps_free(c_json_str);
    }
    
    json_delete(c_obj);
    std::cout << "\n";
    
    // 4. Error handling demonstration
    std::cout << "4. Error handling:\n";
    std::string invalid_json = "{\"invalid\": json}";
    auto invalid_result = jansson::JsonParser::parse(invalid_json);
    
    if (!invalid_result) {
        std::cout << "Caught parse error as expected: " 
                  << invalid_result.error().message() << "\n";
    }
    
    std::cout << "\n=== Demo Complete ===\n";
    
    return 0;
}
