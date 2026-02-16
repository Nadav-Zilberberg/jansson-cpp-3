# Conversion Plan (C -> C++17)

## Overview

This document provides a detailed, step-by-step execution plan to convert the Jansson JSON library from C to modern C++17. The conversion follows the architectural principles outlined in `plan_overview.md` and is organized into phases that respect dependency ordering.

### Phased Approach

The conversion is divided into phases to:
1. **Establish foundational components first** - Create core infrastructure that other modules depend on
2. **Build incrementally** - Each phase introduces components that depend only on previously completed phases
3. **Minimize risk** - Early phases focus on low-risk, leaf modules; later phases integrate complex dependencies
4. **Enable testing** - Complete functional units are available at each phase boundary

### Phase Boundaries

Phase boundaries are defined by:
- **Dependency depth**: Early phases contain modules with no internal dependencies
- **Criticality**: Foundational components (memory management, core types) come first
- **Complexity**: Simpler, self-contained modules precede complex, interconnected ones
- **Testability**: Each phase produces verifiable, functional components

### Dependency Flow

Dependencies flow from early to later phases as follows:
- Phase 1: Utility and leaf modules (no internal deps)
- Phase 2: Core data structures (depend on utilities)
- Phase 3: JSON value representation (depends on core structures)
- Phase 4: Parsing and serialization (depends on JSON values)
- Phase 5: API layer and integration (depends on all below)

## Files to Exclude

The following C source files should NOT be converted to C++ as they implement functionality already available in the C++ standard library or are redundant:

### strbuffer.c / strbuffer.h
**Reason**: The string buffer implementation provides dynamic string manipulation that is fully covered by `std::string` in C++. The C implementation handles:
- Dynamic memory allocation for strings
- Append operations
- Buffer management

**C++ Replacement**: Use `std::string` throughout the codebase. It provides:
- Automatic memory management (RAII)
- Efficient append operations (`+=`, `append()`)
- Built-in buffer management
- Exception safety
- Standard library integration

**Migration Strategy**: Replace all `strbuffer_t` usage with `std::string`. Functions like `strbuffer_init()` become `std::string` constructors, `strbuffer_append_bytes()` becomes `std::string::append()`, and `strbuffer_close()` is handled automatically by destructors.

### hashtable.c / hashtable.h
**Reason**: The custom hash table implementation can be replaced with `std::unordered_map` from the C++ standard library. The C implementation provides:
- Key-value storage
- Hashing functions
- Collision resolution
- Iteration capabilities

**C++ Replacement**: Use `std::unordered_map` with:
- Automatic memory management
- Type safety through templates
- Standardized interface
- Better performance characteristics in most cases
- Integration with STL algorithms

**Migration Strategy**: Replace `hashtable_t` with `std::unordered_map`. The custom hash functions can be adapted to work with `std::hash` specializations if needed.

### memory.c
**Reason**: The memory management functions (`jsonp_malloc`, `jsonp_free`, etc.) are wrappers around standard C allocation functions. In C++, memory management should be handled through:
- RAII principles
- Standard library containers (`std::string`, `std::vector`, etc.)
- Smart pointers (`std::unique_ptr`, `std::shared_ptr`)
- Automatic storage duration for local objects

**C++ Replacement**: Eliminate manual memory management by:
- Using standard library containers that manage their own memory
- Applying RAII to resource ownership
- Using smart pointers for shared ownership scenarios
- Leveraging automatic destructors for cleanup

**Migration Strategy**: Remove custom memory functions and replace with appropriate C++ memory management patterns. Reference counting (used in `json_t`) should be replaced with `std::shared_ptr`.

### utf.c / utf.h
**Reason**: While UTF-8 handling is important, much of this functionality is available in:
- C++20's `<unicode>` support (char8_t, etc.)
- Standard library functions for string manipulation
- Third-party libraries like ICU (if advanced Unicode support is needed)

**C++ Replacement**: For basic UTF-8 validation and manipulation:
- Use `std::string` with UTF-8 encoded content
- Implement validation functions as needed using standard string operations
- Consider `std::u8string` in C++20 for explicit UTF-8 handling

**Migration Strategy**: Keep UTF-8 validation logic but integrate it as methods on a `JsonString` class rather than standalone functions.

### dtoa.c
**Reason**: This file implements floating-point to string conversion. While it's a high-quality implementation, C++17 provides:
- `std::to_chars` for efficient numeric conversion (C++17)
- `std::stringstream` for formatted output
- Type-safe formatting alternatives

**C++ Replacement**: Use `std::to_chars` for performance-critical paths and `std::stringstream` for formatted output. For full C++20 support, consider `<format>`.

**Migration Strategy**: Replace custom dtoa calls with standard library functions, wrapping them in a utility class if needed for consistent formatting.

## Setup

### Core Configuration Changes

**File: CMakeLists.txt**
- Add C++17 standard requirement
- Create separate target for C++ library
- Configure include directories for both C and C++

```cmake
# Add at the top
cmake_minimum_required(VERSION 3.10)
project(jansson VERSION 2.14.1 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create C++ library target
add_library(jansson_cpp STATIC
    src/json_value.cpp
    src/json_object.cpp
    # ... other C++ source files
)

target_include_directories(jansson_cpp
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/c_src
)
```

**File: Makefile.am**
- Add C++ compiler flags
- Create build rules for C++ sources
- Maintain separate build outputs for C and C++

```makefile
AM_CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic
libjansson_cpp_la_SOURCES = \
    src/json_value.cpp \
    src/json_object.cpp \
    # ... other C++ sources
```

### Build System Integration

Create a parallel build structure that:
1. Maintains the original C library in `c_src/`
2. Builds the new C++ library in `src/`
3. Allows incremental testing of C++ components
4. Supports hybrid builds during transition

**Directory Structure:**
```
/
├── c_src/          # Original C sources (preserved)
├── src/             # New C++ sources
├── include/         # Public headers (C compatibility)
└── cpp/include/     # C++-only headers
```

## Phase 1: Utility Layer (No Internal Dependencies)

**Phase Goal**: Implement foundational utility components that have no dependencies on other library modules. These provide basic functionality needed by higher-level components.

**Entry Conditions**:
- C++17 build environment configured
- Basic project structure in place
- All C sources preserved in `c_src/`

**Exit Conditions**:
- All utility classes compiled and tested
- No dependencies on other library components
- Ready for core data structure implementation

### Step 1: Error Handling System

**File**: `src/json_error.cpp`, `src/json_error.hpp`

**Purpose**: Replace C-style error codes with a type-safe error handling system that supports exceptions and error codes.

**Deps**: None (standard library only)

**OOP to apply**:
- Encapsulation: Hide error implementation details
- RAII: Automatic error state management
- Polymorphism: Support multiple error types

**Modern C++ changes**:
- Create `json_error` class hierarchy using `std::error_code` and `std::error_condition`
- Implement `json_exception` derived from `std::runtime_error`
- Provide `Result<T>` template for functional-style error handling
- Use `enum class` for strongly-typed error codes
- Implement `<<` operator for error streaming

**Verification**:
- Compile error handling code in isolation
- Test error code creation and comparison
- Verify exception throwing and catching
- Test `Result<T>` monadic operations (map, and_then)

**Replaces**: Error code definitions in `jansson.h`

### Step 2: String Utilities

**File**: `src/string_utils.cpp`, `src/string_utils.hpp`

**Purpose**: Provide UTF-8 aware string manipulation utilities that replace C string functions.

**Deps**: None (standard library only)

**OOP to apply**:
- Encapsulation: Hide UTF-8 validation internals
- SRP: Single responsibility for string operations

**Modern C++ changes**:
- Create `JsonString` class wrapping `std::string` with UTF-8 guarantees
- Implement static methods for validation: `is_valid_utf8()`
- Provide string manipulation helpers: `escape()`, `unescape()`
- Use `std::string_view` for read-only string parameters
- Implement range-based operations

**Verification**:
- Test UTF-8 validation with valid and invalid sequences
- Test string escaping/unescaping
- Verify `std::string_view` usage doesn't cause dangling references
- Test move semantics for efficient string transfer

**Replaces**: UTF-8 functions in `utf.c`, string manipulation in various files

### Step 3: Memory Management Policy

**File**: `src/memory_policy.hpp`

**Purpose**: Define memory management policies and allocator concepts for the library.

**Deps**: None (standard library only)

**OOP to apply**:
- Strategy Pattern: Different allocation strategies
- RAII: Automatic resource cleanup

**Modern C++ changes**:
- Define `JsonAllocator` concept with `allocate()` and `deallocate()` methods
- Provide default allocator using `std::allocator`
- Create `ArenaAllocator` for bulk allocations
- Implement `JsonPtr<T>` template for resource management
- Use `std::unique_ptr` and `std::shared_ptr` where appropriate

**Verification**:
- Test allocator concept compliance
- Verify memory leak absence with valgrind/ASAN
- Test arena allocator performance characteristics
- Verify exception safety in allocation paths

**Replaces**: Custom memory functions in `memory.c`

## Phase 2: Core Data Structures (Depend on Utilities)

**Phase Goal**: Implement fundamental data structures that form the backbone of JSON representation, depending only on utility components from Phase 1.

**Entry Conditions**:
- All Phase 1 utilities compiled and tested
- Error handling system operational
- String utilities available

**Exit Conditions**:
- Core data structures compiled and unit tested
- JSON value representation foundation in place
- Ready for full JSON type implementation

### Step 4: Hash Table Implementation

**File**: `src/json_hash.cpp`, `src/json_hash.hpp`

**Purpose**: Replace custom hash table with C++ implementation for JSON object storage.

**Deps**: `json_error.hpp`, `string_utils.hpp`

**OOP to apply**:
- Encapsulation: Hide hash table implementation
- Generic Programming: Template on value type
- RAII: Automatic memory management

**Modern C++ changes**:
- Create `JsonHash` class template wrapping `std::unordered_map`
- Specialize `std::hash` for JSON string keys
- Implement custom equality comparison for JSON strings
- Provide iterator range methods (`begin()`/`end()`)
- Add `emplace()` methods for efficient insertion

**Verification**:
- Test insertion, lookup, and deletion operations
- Verify hash collision handling
- Test with different key types
- Measure performance against original implementation

**Replaces**: `hashtable.c` and `hashtable.h`

### Step 5: JSON Value Base Class

**File**: `src/json_value.cpp`, `src/json_value.hpp`

**Purpose**: Create the foundational JSON value class hierarchy that replaces the C `json_t` struct.

**Deps**: `json_error.hpp`, `memory_policy.hpp`

**OOP to apply**:
- Polymorphism: Base class with derived types
- RAII: Automatic reference counting elimination
- Visitor Pattern: Type-safe value access

**Modern C++ changes**:
- Create abstract `JsonValue` base class with virtual interface
- Use `std::variant` for type-safe value storage (C++17)
- Implement `JsonNull`, `JsonBoolean`, `JsonNumber`, `JsonString`, `JsonArray`, `JsonObject` derived classes
- Replace reference counting with `std::shared_ptr<JsonValue>`
- Provide `visit()` method using `std::visit`

**Verification**:
- Test polymorphic behavior
- Verify type-safe value access
- Test reference counting elimination
- Validate `std::variant` usage patterns

**Replaces**: `json_t` struct and basic operations in `value.c`

## Phase 3: JSON Type Implementations (Depend on Core Structures)

**Phase Goal**: Implement concrete JSON type representations that depend on core data structures from Phase 2.

**Entry Conditions**:
- Core data structures (JsonHash, JsonValue) operational
- Memory management policy in place
- Error handling integrated

**Exit Conditions**:
- All JSON types fully implemented
- Type-specific operations available
- Ready for parsing/serialization implementation

### Step 6: JSON Array Implementation

**File**: `src/json_array.cpp`, `src/json_array.hpp`

**Purpose**: Implement JSON array type with vector semantics.

**Deps**: `json_value.hpp`, `json_error.hpp`

**OOP to apply**:
- Encapsulation: Hide storage implementation
- SRP: Single responsibility for array operations
- RAII: Automatic element management

**Modern C++ changes**:
- Derive `JsonArray` from `JsonValue`
- Use `std::vector<std::shared_ptr<JsonValue>>` for storage
- Implement range-based methods (`begin()`/`end()`)
- Provide STL-like interface (`push_back()`, `pop_back()`, `insert()`, `erase()`)
- Add `emplace_back()` for efficient construction

**Verification**:
- Test array creation and element access
- Verify iterator validity
- Test move semantics
- Validate exception safety

**Replaces**: Array-specific functions in `value.c`

### Step 7: JSON Object Implementation

**File**: `src/json_object.cpp`, `src/json_object.hpp`

**Purpose**: Implement JSON object type using the hash table from Phase 2.

**Deps**: `json_value.hpp`, `json_hash.hpp`, `string_utils.hpp`

**OOP to apply**:
- Encapsulation: Hide key-value storage
- SRP: Single responsibility for object operations
- RAII: Automatic key and value management

**Modern C++ changes**:
- Derive `JsonObject` from `JsonValue`
- Use `JsonHash<std::shared_ptr<JsonValue>>` for storage
- Implement `at()` method with bounds checking
- Provide `contains()` method (C++20 style)
- Add range-based iteration over key-value pairs

**Verification**:
- Test object creation and key-value operations
- Verify UTF-8 key handling
- Test iterator validity
- Validate exception safety on invalid keys

**Replaces**: Object-specific functions in `value.c`

### Step 8: JSON String Implementation

**File**: `src/json_string.cpp`, `src/json_string.hpp`

**Purpose**: Implement JSON string type with UTF-8 validation.

**Deps**: `json_value.hpp`, `string_utils.hpp`

**OOP to apply**:
- Encapsulation: Hide string storage
- SRP: Single responsibility for string operations
- Immutable Pattern: JSON strings are immutable

**Modern C++ changes**:
- Derive `JsonString` from `JsonValue`
- Store data in `std::string` with UTF-8 validation on construction
- Provide `std::string_view` accessor
- Implement comparison operators
- Add `c_str()` method for C compatibility

**Verification**:
- Test UTF-8 validation on construction
- Verify immutability guarantees
- Test comparison operations
- Validate C compatibility

**Replaces**: String-specific functions in `value.c`

### Step 9: JSON Number Implementation

**File**: `src/json_number.cpp`, `src/json_number.hpp`

**Purpose**: Implement JSON number type with integer and real support.

**Deps**: `json_value.hpp`, `json_error.hpp`

**OOP to apply**:
- Encapsulation: Hide numeric representation
- Type Safety: Distinguish integer and real types

**Modern C++ changes**:
- Derive `JsonInteger` and `JsonReal` from `JsonValue`
- Use `std::variant<int64_t, double>` for storage
- Provide type-safe accessors (`as_integer()`, `as_real()`)
- Implement numeric comparison operators
- Use `std::to_chars` for string conversion

**Verification**:
- Test integer and real number operations
- Verify type-safe access
- Test numeric comparisons
- Validate string conversion accuracy

**Replaces**: Number-specific functions in `value.c`

## Phase 4: Parsing and Serialization (Depend on JSON Types)

**Phase Goal**: Implement JSON parsing from strings and serialization to strings, depending on complete JSON type system from Phase 3.

**Entry Conditions**:
- All JSON types (Array, Object, String, Number) fully implemented
- Core data structures operational
- Error handling integrated

**Exit Conditions**:
- Parser can handle valid JSON input
- Serializer produces valid JSON output
- Round-trip conversion works correctly
- Ready for API layer implementation

### Step 10: JSON Parser Implementation

**File**: `src/json_parser.cpp`, `src/json_parser.hpp`

**Purpose**: Implement JSON parser that converts string input to JSON value objects.

**Deps**: All JSON value types, `json_error.hpp`, `string_utils.hpp`

**OOP to apply**:
- Encapsulation: Hide parsing state
- SRP: Single responsibility for parsing
- State Pattern: Different parsing states

**Modern C++ changes**:
- Create `JsonParser` class with `parse()` method
- Use recursive descent parsing with state machine
- Implement `std::string_view` for zero-copy parsing
- Provide detailed error reporting with position information
- Use `std::optional<JsonValue>` for parse results

**Verification**:
- Test parsing of all JSON value types
- Verify error reporting accuracy
- Test edge cases (empty objects, nested structures)
- Validate performance characteristics

**Replaces**: `load.c` parsing functions

### Step 11: JSON Serializer Implementation

**File**: `src/json_serializer.cpp`, `src/json_serializer.hpp`

**Purpose**: Implement JSON serializer that converts JSON value objects to string output.

**Deps**: All JSON value types, `string_utils.hpp`

**OOP to apply**:
- Encapsulation: Hide serialization details
- SRP: Single responsibility for serialization
- Visitor Pattern: Type-specific serialization

**Modern C++ changes**:
- Create `JsonSerializer` class with `serialize()` method
- Use `std::string` for output
- Implement pretty-printing options
- Provide customization points for formatting
- Use `std::visit` on `std::variant` for type handling

**Verification**:
- Test serialization of all JSON value types
- Verify pretty-printing options
- Test round-trip with parser
- Validate escape sequence handling

**Replaces**: `dump.c` serialization functions

## Phase 5: API Layer and Integration (Depend on All Below)

**Phase Goal**: Implement the public C-compatible API layer and integrate all components into a cohesive library.

**Entry Conditions**:
- All JSON types implemented
- Parser and serializer operational
- Core utilities available

**Exit Conditions**:
- C-compatible API fully functional
- All original functionality preserved
- Library ready for production use

### Step 12: C-Compatible API Facade

**File**: `src/jansson.cpp`, `include/jansson.h`

**Purpose**: Provide C-compatible API that forwards to C++ implementation.

**Deps**: All internal C++ components

**OOP to apply**:
- Facade Pattern: Hide C++ implementation
- Adapter Pattern: Convert between C and C++ types

**Modern C++ changes**:
- Create `extern "C"` functions matching original API
- Convert C `json_t*` to `std::shared_ptr<JsonValue>` internally
- Implement reference counting using `std::shared_ptr`
- Provide C-style error handling while using C++ exceptions internally
- Maintain binary compatibility where possible

**Verification**:
- Test all original API functions
- Verify reference counting behavior
- Validate error code compatibility
- Test memory management across C/C++ boundary

**Replaces**: Public API in original `jansson.h`

### Step 13: Pack/Unpack Implementation

**File**: `src/json_pack.cpp`, `src/json_pack.hpp`

**Purpose**: Implement format string-based JSON construction and decomposition.

**Deps**: All JSON value types, parser, serializer

**OOP to apply**:
- Interpreter Pattern: Parse format strings
- Builder Pattern: Construct JSON values

**Modern C++ changes**:
- Create `JsonPacker` class for construction
- Create `JsonUnpacker` class for decomposition
- Use `std::variant` for type-safe value extraction
- Implement format string parsing with error checking
- Provide type-safe accessors

**Verification**:
- Test all format specifiers
- Verify error handling
- Test edge cases
- Validate memory safety

**Replaces**: `pack_unpack.c` functions

## C-to-C++ Conversion Rules and Patterns

### Struct-to-Class Mappings

**Rule**: Convert C structs to proper C++ classes with encapsulation.

**Pattern**:
```cpp
// C struct
typedef struct {
    json_type type;
    volatile size_t refcount;
} json_t;

// C++ class
class JsonValue {
public:
    explicit JsonValue(json_type type);
    virtual ~JsonValue() = default;
    
    json_type type() const noexcept { return type_; }
    
    // No direct refcount access - use std::shared_ptr instead
private:
    json_type type_;
    // refcount managed by std::shared_ptr
};
```

**Rationale**:
- Encapsulate implementation details
- Eliminate manual reference counting
- Provide controlled access to members
- Enable polymorphic behavior

### Function-to-Method Transformations

**Rule**: Convert free functions operating on structs to member methods.

**Pattern**:
```cpp
// C function
json_t *json_object(void);
int json_object_set(json_t *object, const char *key, json_t *value);

// C++ methods
class JsonObject : public JsonValue {
public:
    static std::shared_ptr<JsonObject> create();
    
    bool set(const std::string &key, std::shared_ptr<JsonValue> value);
    std::shared_ptr<JsonValue> get(const std::string &key) const;
};
```

**Rationale**:
- Improve code organization
- Enable method chaining
- Provide better encapsulation
- Support operator overloading

### Ownership and Lifetime Management

**Rule**: Replace manual memory management with RAII principles.

**Pattern**:
```cpp
// C - manual memory management
json_t *obj = json_object();
json_object_set(obj, "key", json_string("value"));
// ... use obj ...
json_decref(obj); // easy to forget!

// C++ - RAII
auto obj = JsonObject::create();
obj->set("key", JsonString::create("value"));
// ... use obj ...
// Automatic cleanup when obj goes out of scope
```

**Rationale**:
- Prevent memory leaks
- Eliminate use-after-free bugs
- Reduce boilerplate code
- Improve exception safety

### Standard Library Types

**Rule**: Prefer standard library types over custom implementations.

**Mappings**:
- `strbuffer_t` → `std::string`
- `hashtable_t` → `std::unordered_map`
- Manual arrays → `std::vector` or `std::array`
- C strings → `std::string` or `std::string_view`
- Custom allocators → `std::allocator` or custom allocators

**Example**:
```cpp
// C - custom string buffer
strbuffer_t buf;
strbuffer_init(&buf);
strbuffer_append_bytes(&buf, "hello", 5);
// ...
strbuffer_close(&buf);

// C++ - standard string
std::string str;
str.append("hello", 5);
// Automatic cleanup
```

### Modern C++ Constructs

**Rule**: Leverage C++17 features for safety and expressiveness.

**Constructs to Use**:
- `std::string_view` for read-only string parameters
- `std::optional<T>` for nullable values
- `std::variant<T...>` for type-safe unions
- `std::any` sparingly for type-erased storage
- Structured bindings for tuple-like returns
- `if constexpr` for compile-time branching
- Fold expressions for variadic templates
- `[[nodiscard]]` for functions that shouldn't be ignored

**Example**:
```cpp
// Using std::optional for nullable JSON values
std::optional<JsonValue> parse_json(std::string_view input) noexcept;

// Using std::variant for JSON value types
using JsonValueVariant = std::variant<
    std::monostate,       // null
    bool,                 // boolean
    int64_t,              // integer
    double,               // real
    std::string,          // string
    JsonArray,            // array
    JsonObject            // object
>;

// Using structured bindings
auto [value, error] = JsonParser::parse(input);
if (!error) {
    // use value
}
```

### Error Handling

**Rule**: Use exceptions for unrecoverable errors, `std::optional`/`std::expected` for recoverable errors.

**Pattern**:
```cpp
// For parsing errors (recoverable)
std::optional<JsonValue> JsonParser::parse(std::string_view input) {
    try {
        // parsing logic
        return value;
    } catch (const ParseError &e) {
        return std::nullopt;
    }
}

// For programming errors (unrecoverable)
JsonValue &JsonObject::at(const std::string &key) {
    if (!contains(key)) {
        throw std::out_of_range("Key not found: " + key);
    }
    return *values_.at(key);
}
```

**Rationale**:
- Distinguish between recoverable and unrecoverable errors
- Provide better error information
- Enable proper resource cleanup
- Support idiomatic C++ error handling

### Const-Correctness

**Rule**: Use `const` extensively to enforce immutability guarantees.

**Pattern**:
```cpp
class JsonObject {
public:
    // Const method - doesn't modify object
    std::shared_ptr<const JsonValue> get(const std::string &key) const;
    
    // Non-const method - modifies object
    void set(const std::string &key, std::shared_ptr<JsonValue> value);
    
    // Const iterator methods
    auto begin() const { return values_.begin(); }
    auto end() const { return values_.end(); }
    
private:
    std::unordered_map<std::string, std::shared_ptr<JsonValue>> values_;
};
```

**Rationale**:
- Enforce compile-time immutability
- Enable const-propagation
- Improve code safety
- Support const-correct generic code

### Move Semantics

**Rule**: Implement move constructors and move assignment for efficient resource transfer.

**Pattern**:
```cpp
class JsonString : public JsonValue {
public:
    // Copy constructor
    JsonString(const std::string &value);
    
    // Move constructor
    JsonString(std::string &&value) noexcept;
    
    // Copy assignment
    JsonString &operator=(const JsonString &other);
    
    // Move assignment
    JsonString &operator=(JsonString &&other) noexcept;
    
private:
    std::string value_;
};

// Efficient transfer
JsonString create_string() {
    std::string temp = "large string...";
    return JsonString(std::move(temp)); // efficient move
}
```

**Rationale**:
- Avoid unnecessary copies
- Improve performance
- Enable efficient return value optimization
- Support move-only types
