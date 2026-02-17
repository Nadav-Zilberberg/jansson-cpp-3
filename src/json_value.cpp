#include "json_value.hpp"
#include "json_hash.hpp"
#include "string_utils.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace jansson {

// JsonNull implementation
bool JsonNull::equals(const JsonValue& other) const noexcept {
    return other.is_null();
}

std::shared_ptr<JsonValue> JsonNull::clone() const {
    return create();
}

// JsonBoolean implementation
bool JsonBoolean::equals(const JsonValue& other) const noexcept {
    return other.is_boolean() && other.boolean_value() == value_;
}

std::shared_ptr<JsonValue> JsonBoolean::clone() const {
    return create(value_);
}

// JsonNumber implementation
std::string JsonNumber::to_string() const {
    // Check if the number is an integer
    if (std::floor(value_) == value_) {
        return std::to_string(static_cast<int64_t>(value_));
    }
    
    // Use default formatting for floating point
    std::ostringstream oss;
    oss << value_;
    return oss.str();
}

bool JsonNumber::equals(const JsonValue& other) const noexcept {
    return other.is_number() && 
           std::abs(other.number_value() - value_) < 1e-12; // Allow for floating point tolerance
}

std::shared_ptr<JsonValue> JsonNumber::clone() const {
    return create(value_);
}

// JsonStringValue implementation
JsonStringValue::JsonStringValue(const std::string& value)
    : value_(value) {}

JsonStringValue::JsonStringValue(std::string&& value)
    : value_(std::move(value)) {}

JsonStringValue::JsonStringValue(std::string_view value)
    : value_(value) {}

std::string JsonStringValue::to_string() const {
    return JsonString::escape(value_);
}

bool JsonStringValue::equals(const JsonValue& other) const noexcept {
    return other.is_string() && other.string_value() == value_;
}

std::shared_ptr<JsonValue> JsonStringValue::clone() const {
    return create(value_);
}

// JsonArray implementation
std::shared_ptr<JsonValue> JsonArray::at(size_t index) const {
    if (index >= values_.size()) {
        throw JsonException("Array index out of bounds");
    }
    return values_[index];
}

std::string JsonArray::to_string() const {
    std::ostringstream oss;
    oss << "[";
    
    for (size_t i = 0; i < values_.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << values_[i]->to_string();
    }
    
    oss << "]";
    return oss.str();
}

bool JsonArray::equals(const JsonValue& other) const noexcept {
    if (!other.is_array()) {
        return false;
    }
    
    const auto& other_array = other.array_value();
    if (values_.size() != other_array.size()) {
        return false;
    }
    
    for (size_t i = 0; i < values_.size(); ++i) {
        if (!values_[i]->equals(*other_array[i])) {
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<JsonValue> JsonArray::clone() const {
    auto result = create();
    for (const auto& value : values_) {
        result->push_back(value->clone());
    }
    return result;
}

// JsonObject implementation
void JsonObject::set(const std::string& key, std::shared_ptr<JsonValue> value) {
    values_[key] = std::move(value);
}

std::shared_ptr<JsonValue> JsonObject::get(const std::string& key) const {
    auto it = values_.find(key);
    if (it == values_.end()) {
        return nullptr;
    }
    return it->second;
}

bool JsonObject::has(const std::string& key) const {
    return values_.contains(key);
}

void JsonObject::erase(const std::string& key) {
    values_.erase(key);
}

std::string JsonObject::to_string() const {
    std::ostringstream oss;
    oss << "{";
    
    bool first = true;
    for (const auto& [key, value] : values_) {
        if (!first) {
            oss << ", ";
        }
        oss << JsonString::escape(key) << ": " << value->to_string();
        first = false;
    }
    
    oss << "}";
    return oss.str();
}

bool JsonObject::equals(const JsonValue& other) const noexcept {
    if (!other.is_object()) {
        return false;
    }
    
    const auto& other_obj = other.object_value();
    if (values_.size() != other_obj.size()) {
        return false;
    }
    
    for (const auto& [key, value] : values_) {
        auto it = other_obj.find(key);
        if (it == other_obj.end() || !value->equals(*it->second)) {
            return false;
        }
    }
    
    return true;
}

std::shared_ptr<JsonValue> JsonObject::clone() const {
    auto result = create();
    for (const auto& [key, value] : values_) {
        result->set(key, value->clone());
    }
    return result;
}

// Default implementations that throw
bool JsonValue::boolean_value() const {
    throw JsonException("Value is not a boolean");
}

double JsonValue::number_value() const {
    throw JsonException("Value is not a number");
}

const std::string& JsonValue::string_value() const {
    throw JsonException("Value is not a string");
}

const std::vector<std::shared_ptr<JsonValue>>& JsonValue::array_value() const {
    throw JsonException("Value is not an array");
}

const JsonHash<std::string, std::shared_ptr<JsonValue>>& JsonValue::object_value() const {
    throw JsonException("Value is not an object");
}

} // namespace jansson
