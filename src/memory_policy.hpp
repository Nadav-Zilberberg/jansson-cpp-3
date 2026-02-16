#ifndef MEMORY_POLICY_HPP
#define MEMORY_POLICY_HPP

#include <memory>
#include <vector>
#include <unordered_map>

namespace jansson {

// Allocator concept for JSON library
template <typename T>
class JsonAllocator {
public:
    using value_type = T;
    
    JsonAllocator() = default;
    template <typename U>
    JsonAllocator(const JsonAllocator<U>&) noexcept {}
    
    T* allocate(std::size_t n) {
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    
    void deallocate(T* p, std::size_t) noexcept {
        ::operator delete(p);
    }
};

// Default allocator using std::allocator
template <typename T>
using DefaultAllocator = std::allocator<T>;

// Arena allocator for bulk allocations
template <typename T>
class ArenaAllocator {
public:
    using value_type = T;
    
    explicit ArenaAllocator(std::size_t block_size = 4096)
        : block_size_(block_size) {}
    
    template <typename U>
    ArenaAllocator(const ArenaAllocator<U>& other) noexcept
        : block_size_(other.block_size_) {}
    
    T* allocate(std::size_t n) {
        if (n > block_size_) {
            // Fall back to regular allocation for large objects
            return static_cast<T*>(::operator new(n * sizeof(T)));
        }
        
        if (!current_block_ || current_offset_ + n * sizeof(T) > block_size_) {
            // Allocate new block
            current_block_ = static_cast<char*>(::operator new(block_size_));
            current_offset_ = 0;
            blocks_.push_back(current_block_);
        }
        
        T* result = reinterpret_cast<T*>(current_block_ + current_offset_);
        current_offset_ += n * sizeof(T);
        return result;
    }
    
    void deallocate(T*, std::size_t) noexcept {
        // Arena allocator doesn't free individual objects
    }
    
    ~ArenaAllocator() {
        for (char* block : blocks_) {
            ::operator delete(block);
        }
    }
    
    // Reset the arena (deallocates all memory)
    void reset() {
        for (char* block : blocks_) {
            ::operator delete(block);
        }
        blocks_.clear();
        current_block_ = nullptr;
        current_offset_ = 0;
    }

private:
    std::size_t block_size_;
    std::vector<char*> blocks_;
    char* current_block_ = nullptr;
    std::size_t current_offset_ = 0;
};

// Resource management pointer
template <typename T, typename Deleter = std::default_delete<T>>
using JsonPtr = std::unique_ptr<T, Deleter>;

// Shared pointer for shared ownership
template <typename T>
using JsonSharedPtr = std::shared_ptr<T>;

// Weak pointer for non-owning references
template <typename T>
using JsonWeakPtr = std::weak_ptr<T>;

} // namespace jansson

#endif // MEMORY_POLICY_HPP
