#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP

#include <memory>
#include <variant>
#include <vector>
#include <string>
#include <cstdint>
#include "json_error.hpp"
#include "json_hash.hpp"

namespace jansson {

// Forward declarations
class JsonValue;
class JsonNull;
class JsonBoolean;
class JsonNumber;
class JsonStringValue;
class JsonArray;
class JsonObject;

// JSON type enum (matches C API)
enum class JsonType {
    Null,
    Boolean,
    Number,
    String,
    Array,
    Object
};

// Base JSON value class
class JsonValue : public std::enable_shared_from_this<JsonValue> {
public:
    virtual ~JsonValue() = default;
    
    // Get the type of this value
    virtual JsonType type() const noexcept = 0;
    
    // Type checking
    virtual bool is_null() const noexcept { return false; }
    virtual bool is_boolean() const noexcept { return false; }
    virtual bool is_number() const noexcept { return false; }
    virtual bool is_string() const noexcept { return false; }
    virtual bool is_array() const noexcept { return false; }
    virtual bool is_object() const noexcept { return false; }
    
    // Value access (will throw if wrong type)
    virtual bool boolean_value() const;
    virtual double number_value() const;
    virtual const std::string& string_value() const;
    virtual const std::vector<std::shared_ptr<JsonValue>>& array_value() const;
    virtual const JsonHash<std::string, std::shared_ptr<JsonValue>>& object_value() const;
    
    // String representation
    virtual std::string to_string() const = 0;
    
    // Comparison
    virtual bool equals(const JsonValue& other) const noexcept = 0;
    
    // Clone this value
    virtual std::shared_ptr<JsonValue> clone() const = 0;
};

// Null value
class JsonNull : public JsonValue {
public:
    static std::shared_ptr<JsonNull> create() {
        return std::shared_ptr<JsonNull>(new JsonNull());
    }
    
    JsonType type() const noexcept override { return JsonType::Null; }
    bool is_null() const noexcept override { return true; }
    
    std::string to_string() const override { return "null"; }
    bool equals(const JsonValue& other) const noexcept override;
    std::shared_ptr<JsonValue> clone() const override;
};

// Boolean value
class JsonBoolean : public JsonValue {
public:
    explicit JsonBoolean(bool value) : value_(value) {}
    
    static std::shared_ptr<JsonBoolean> create(bool value) {
        return std::shared_ptr<JsonBoolean>(new JsonBoolean(value));
    }
    
    JsonType type() const noexcept override { return JsonType::Boolean; }
    bool is_boolean() const noexcept override { return true; }
    bool boolean_value() const override { return value_; }
    
    std::string to_string() const override { return value_ ? "true" : "false"; }
    bool equals(const JsonValue& other) const noexcept override;
    std::shared_ptr<JsonValue> clone() const override;

private:
    bool value_;
};

// Number value (supports both integer and real)
class JsonNumber : public JsonValue {
public:
    explicit JsonNumber(double value) : value_(value) {}
    
    static std::shared_ptr<JsonNumber> create(double value) {
        return std::shared_ptr<JsonNumber>(new JsonNumber(value));
    }
    
    JsonType type() const noexcept override { return JsonType::Number; }
    bool is_number() const noexcept override { return true; }
    double number_value() const override { return value_; }
    
    std::string to_string() const override;
    bool equals(const JsonValue& other) const noexcept override;
    std::shared_ptr<JsonValue> clone() const override;

private:
    double value_;
};

// String value
class JsonStringValue : public JsonValue {
public:
    explicit JsonStringValue(const std::string& value);
    explicit JsonStringValue(std::string&& value);
    explicit JsonStringValue(std::string_view value);
    
    static std::shared_ptr<JsonStringValue> create(const std::string& value) {
        return std::shared_ptr<JsonStringValue>(new JsonStringValue(value));
    }
    
    static std::shared_ptr<JsonStringValue> create(std::string&& value) {
        return std::shared_ptr<JsonStringValue>(new JsonStringValue(std::move(value)));
    }
    
    JsonType type() const noexcept override { return JsonType::String; }
    bool is_string() const noexcept override { return true; }
    const std::string& string_value() const override { return value_; }
    
    std::string to_string() const override;
    bool equals(const JsonValue& other) const noexcept override;
    std::shared_ptr<JsonValue> clone() const override;

private:
    std::string value_;
};

// Array value
class JsonArray : public JsonValue {
public:
    JsonArray() = default;
    explicit JsonArray(std::vector<std::shared_ptr<JsonValue>> values)
        : values_(std::move(values)) {}
    
    static std::shared_ptr<JsonArray> create() {
        return std::shared_ptr<JsonArray>(new JsonArray());
    }
    
    JsonType type() const noexcept override { return JsonType::Array; }
    bool is_array() const noexcept override { return true; }
    const std::vector<std::shared_ptr<JsonValue>>& array_value() const override { return values_; }
    
    // Array operations
    void push_back(std::shared_ptr<JsonValue> value) {
        values_.push_back(std::move(value));
    }
    
    std::shared_ptr<JsonValue> at(size_t index) const;
    size_t size() const noexcept { return values_.size(); }
    bool empty() const noexcept { return values_.empty(); }
    
    // Iterators
    auto begin() const { return values_.begin(); }
    auto end() const { return values_.end(); }
    
    std::string to_string() const override;
    bool equals(const JsonValue& other) const noexcept override;
    std::shared_ptr<JsonValue> clone() const override;

private:
    std::vector<std::shared_ptr<JsonValue>> values_;
};

// Object value
class JsonObject : public JsonValue {
public:
    JsonObject() = default;
    
    static std::shared_ptr<JsonObject> create() {
        return std::shared_ptr<JsonObject>(new JsonObject());
    }
    
    JsonType type() const noexcept override { return JsonType::Object; }
    bool is_object() const noexcept override { return true; }
    const JsonHash<std::string, std::shared_ptr<JsonValue>>& object_value() const override { return values_; }
    
    // Object operations
    void set(const std::string& key, std::shared_ptr<JsonValue> value);
    std::shared_ptr<JsonValue> get(const std::string& key) const;
    bool has(const std::string& key) const;
    void erase(const std::string& key);
    size_t size() const noexcept { return values_.size(); }
    bool empty() const noexcept { return values_.empty(); }
    
    // Iterators
    auto begin() const { return values_.begin(); }
    auto end() const { return values_.end(); }
    
    std::string to_string() const override;
    bool equals(const JsonValue& other) const noexcept override;
    std::shared_ptr<JsonValue> clone() const override;

private:
    JsonHash<std::string, std::shared_ptr<JsonValue>> values_;
};

} // namespace jansson

#endif // JSON_VALUE_HPP
