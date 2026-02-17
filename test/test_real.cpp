#include <iostream>
#include <cassert>
#include <cmath>
#include "json_c_api.hpp"

int main() {
    std::cout << "Running test_real..." << std::endl;
    
    // Test creating real number
    json_t* real_json = json_real(3.14159);
    assert(real_json != nullptr);
    assert(json_typeof(real_json) == JSON_REAL);
    assert(json_is_number(real_json) == 1);
    
    double value = json_real_value(real_json);
    assert(fabs(value - 3.14159) < 0.00001);
    
    // Test negative real
    json_t* neg_real_json = json_real(-2.71828);
    assert(neg_real_json != nullptr);
    value = json_real_value(neg_real_json);
    assert(fabs(value - (-2.71828)) < 0.00001);
    
    // Test zero
    json_t* zero_real_json = json_real(0.0);
    assert(zero_real_json != nullptr);
    value = json_real_value(zero_real_json);
    assert(fabs(value - 0.0) < 0.00001);
    
    // Test serialization
    char* real_str = json_dumps(real_json, 0);
    assert(real_str != nullptr);
    std::cout << "Real serialized: " << real_str << std::endl;
    
    json_dumps_free(real_str);
    json_delete(real_json);
    json_delete(neg_real_json);
    json_delete(zero_real_json);
    
    std::cout << "test_real passed!" << std::endl;
    return 0;
}
