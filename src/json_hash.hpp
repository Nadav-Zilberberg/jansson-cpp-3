#ifndef JSON_HASH_HPP
#define JSON_HASH_HPP

#include <unordered_map>
#include <string>
#include <functional>

namespace jansson {

// Custom hash function for strings
template <typename T>
struct JsonStringHash {
    std::size_t operator()(const T& key) const {
        return std::hash<T>{}(key);
    }
};

// Custom equality comparison for strings
template <typename T>
struct JsonStringEqual {
    bool operator()(const T& lhs, const T& rhs) const {
        return lhs == rhs;
    }
};

// JSON hash table implementation
template <
    typename Key,
    typename Value,
    typename Hash = JsonStringHash<Key>,
    typename KeyEqual = JsonStringEqual<Key>
>
class JsonHash {
public:
    using key_type = Key;
    using mapped_type = Value;
    using value_type = std::pair<const Key, Value>;
    using size_type = std::size_t;
    using iterator = typename std::unordered_map<Key, Value, Hash, KeyEqual>::iterator;
    using const_iterator = typename std::unordered_map<Key, Value, Hash, KeyEqual>::const_iterator;
    
    // Constructors
    JsonHash() = default;
    explicit JsonHash(std::size_t bucket_count)
        : map_(bucket_count) {}
    
    // Insertion
    std::pair<iterator, bool> insert(const value_type& value) {
        return map_.insert(value);
    }
    
    std::pair<iterator, bool> insert(value_type&& value) {
        return map_.insert(std::move(value));
    }
    
    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        return map_.emplace(std::forward<Args>(args)...);
    }
    
    // Access
    Value& at(const Key& key) {
        return map_.at(key);
    }
    
    const Value& at(const Key& key) const {
        return map_.at(key);
    }
    
    Value& operator[](const Key& key) {
        return map_[key];
    }
    
    // Lookup
    iterator find(const Key& key) {
        return map_.find(key);
    }
    
    const_iterator find(const Key& key) const {
        return map_.find(key);
    }
    
    bool contains(const Key& key) const {
        return map_.find(key) != map_.end();
    }
    
    // Erase
    size_type erase(const Key& key) {
        return map_.erase(key);
    }
    
    iterator erase(iterator pos) {
        return map_.erase(pos);
    }
    
    // Size
    bool empty() const noexcept {
        return map_.empty();
    }
    
    size_type size() const noexcept {
        return map_.size();
    }
    
    // Iteration
    iterator begin() noexcept {
        return map_.begin();
    }
    
    const_iterator begin() const noexcept {
        return map_.begin();
    }
    
    iterator end() noexcept {
        return map_.end();
    }
    
    const_iterator end() const noexcept {
        return map_.end();
    }
    
    // Clear
    void clear() noexcept {
        map_.clear();
    }

private:
    std::unordered_map<Key, Value, Hash, KeyEqual> map_;
};

} // namespace jansson

#endif // JSON_HASH_HPP
