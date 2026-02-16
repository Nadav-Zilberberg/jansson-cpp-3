# Conversion Plan

## Overview
The goal is to migrate the entire `c_src/` C codebase into modern C++17 code located in a sibling directory `cpp/`.  
The migration follows a **dependency‑first, bottom‑up** approach: files that have no unmet internal dependencies are converted first, and each subsequent phase depends on the successful conversion of the previous phase.  
Each conversion step explicitly states:

* **Purpose** – what the module does.  
* **Internal dependencies** – other C files that must be converted before it.  
* **OOP principles** to apply (Encapsulation, SRP, Abstraction, Composition, Polymorphism, RAII).  
* **Modern C++ changes** – concrete replacements (smart pointers, `enum class`, STL containers, `constexpr`, etc.).  
* **Public API outcome** – the new class/function surface that callers will use.  
* **Tests/verification** – behavior to validate after conversion.

The plan is divided into **phases**, each containing **multiple steps** (2‑3 files per phase whenever possible). Phases represent logical layers of the library, from low‑level utilities to higher‑level JSON handling.

---

## Phase 1 – Leaf Utilities (Zero Internal Dependencies)

### Step 1 – Convert `version.c` → `cpp/version.cpp`
- **Purpose**: Exposes the library version string (`JSON_VERSION`) to callers.  
- **Internal deps**: none.  
- **OOP principles to apply**:
  - **Encapsulation** – wrap the version string in a `constexpr` class member; hide the raw macro.  
  - **SRP** – a dedicated `Version` class with a single responsibility (provide version info).  
  - **RAII** – no resource acquisition needed; use `constexpr` to guarantee immutability.  
- **Modern C++ changes**:
  - Replace the C macro with a `constexpr` variable inside a `Version` class.  
  - Use `enum class` for any version‑related enum (if introduced).  
  - Return a `std::string_view` for the version string.  
- **Public API outcome**: `class Version { public: static constexpr std::string_view String(); };`  
- **Tests/verification**: Ensure the returned string matches the original `JSON_VERSION` constant.

### Step 2 – Convert `error.c` → `cpp/error.cpp`
- **Purpose**: Implements error codes and messages used throughout the library.  
- **Internal deps**: none.  
- **OOP principles to apply**:
  - **Encapsulation** – map each error code to a private enum class; expose a public `Error` class with descriptive methods.  
  - **SRP** – a single class managing all error-related functionality.  
  - **RAII** – associate error messages with exception objects that can be thrown.  
- **Modern C++ changes**:
  - Replace raw `int` error codes with a scoped `enum class ErrorCode`.  
  - Convert `const char*` messages to `std::string_view`.  
  - Throw `std::runtime_error` (or a custom `JanssonException`) instead of returning error codes where appropriate.  
- **Public API outcome**: `class JanssonException : public std::runtime_error { public: static ErrorCode Code(); };`  
- **Tests/verification**: Verify that existing error‑code checks still compile and that exception messages are equivalent.

### Step 3 – Convert `utf.c` → `cpp/utf.cpp`
- **Purpose**: Provides UTF‑8/UTF‑16 helper functions (e.g., validation, conversion).  
- **Internal deps**: none.  
- **OOP principles to apply**:
  - **Encapsulation** – hide C‑style functions behind a namespace‑level `Utf` class.  
  - **SRP** – each helper (e.g., `IsValidUtf8`) belongs to a focused method.  
  - **RAII** – use `std::string_view` for input buffers; no manual allocation.  
- **Modern C++ changes**:
  - Replace raw `const char*` buffers with `std::string_view`.  
  - Use `std::optional` for functions that may fail.  
  - Leverage `<codecvt>` or `std::u8string_literal` where appropriate.  
- **Public API outcome**: `namespace Utf { bool IsValidUtf8(std::string_view); }`  
- **Tests/verification**: Run existing UTF‑validation tests; confirm identical boolean results.

---

## Phase 2 – String Handling Utilities

### Step 4 – Convert `strconv.c` → `cpp/strconv.cpp`
- **Purpose**: Implements conversion utilities between strings and numbers (e.g., `strtod`, `dopr`).  
- **Internal deps**: `version.c` (for version string), `error.c` (error handling).  
- **OOP principles to apply**:
  - **Encapsulation** – wrap conversion functions in a `StrConv` class.  
  - **SRP** – each conversion (e.g., `ToDouble`) is a separate method.  
  - **Composition** – use `Utf` utilities for UTF‑aware parsing.  
- **Modern C++ changes**:
  - Replace `char*` buffers with `std::string_view`.  
  - Use `std::from_chars` / `std::to_chars` (C++17) for locale‑independent conversions.  
  - Return `std::optional<double>` on failure instead of error codes.  
- **Public API outcome**: `class StrConv { public: static std::optional<double> ToDouble(std::string_view); };`  
- **Tests/verification**: Compare conversion results against the original C implementation on a set of sample inputs.

### Step 5 – Convert `strbuffer.c` → `cpp/strbuffer.cpp`
- **Purpose**: Implements a dynamic string buffer used by `dump.c` and other modules.  
- **Internal deps**: `error.c` (error handling), `utf.c` (UTF utilities).  
- **OOP principles to apply**:
  - **Encapsulation** – expose a `StrBuffer` class that manages internal storage.  
  - **SRP** – the class only handles buffer growth, append, and finalization.  
  - **RAII** – memory is owned by the class; no manual `malloc`/`free`.  
- **Modern C++ changes**:
  - Replace internal `char*` with `std::vector<char>` or `std::string`.  
  - Use `reserve()` and `push_back()` for growth; `std::string_view` for slices.  
  - Provide `std::string Release();` to obtain the final owned string.  
- **Public API outcome**: `class StrBuffer { public: StrBuffer(); void Append(std::string_view); std::string Release(); };`  
- **Tests/verification**: Ensure that output strings produced after conversion are byte‑identical to the original C version.

---

## Phase 3 – Hash Table and Seed Utilities

### Step 6 – Convert `hashtable_seed.c` → `cpp/hashtable_seed.cpp`
- **Purpose**: Provides a pseudo‑random seed generator used by the hash table implementation.  
- **Internal deps**: `hashtable.c` (uses the seed).  
- **OOP principles to apply**:
  - **Encapsulation** – wrap seed logic in a `HashSeed` class.  
  - **SRP** – single responsibility for seed generation.  
  - **RAII** – no external resources; use `std::mt19937` with deterministic initialization.  
- **Modern C++ changes**:
  - Replace `rand()`/`srand()` with `std::mt19937` seeded from `std::random_device`.  
  - Return a `uint64_t` directly; no C‑style buffers.  
- **Public API outcome**: `class HashSeed { public: static uint64_t Generate(); };`  
- **Tests/verification**: Validate that the generated seed matches the original algorithm’s output for known inputs.

### Step 7 – Convert `hashtable.c` → `cpp/hashtable.cpp`
- **Purpose**: Core hash table implementation used by the JSON parser for object storage.  
- **Internal deps**: `hashtable_seed.c` (seed), `error.c` (error handling), `hashtable.h` (public API).  
- **OOP principles to apply**:
  - **Encapsulation** – hide the internal bucket array behind a `HashTable` class.  
  - **SRP** – each method (Insert, Find, Erase) has a clear purpose.  
  - **Composition** – use `HashSeed` for randomness; use `std::vector<std::unique_ptr<Node>>` for buckets.  
  - **RAII** – manage node memory with `std::unique_ptr`.  
- **Modern C++ changes**:
  - Replace raw `struct json_object *` pointers with `std::unique_ptr<Node>`.  
  - Use `enum class BucketState { Empty, Occupied, Deleted };` instead of plain enums.  
  - Replace `void *` generic pointers with templates (`std::unique_ptr<T>`).  
  - Use `std::unordered_map`‑like interface but retain custom behavior for JSON semantics.  
- **Public API outcome**: `class HashTable { public: void Insert(std::string_view key, json_value* val); json_value* Find(std::string_view key) const; };`  
- **Tests/verification**: Run the existing hash‑table test suite; confirm identical collision behavior and lookup results.

---

## Phase 4 – Core JSON Value Representation

### Step 8 – Convert `value.c` → `cpp/value.cpp`
- **Purpose**: Defines the `json_value` structure and associated operations (object, array, number, boolean, null).  
- **Internal deps**: `hashtable.c` (stores object members), `strbuffer.c` (for string representation), `error.c` (error handling).  
- **OOP principles to apply**:
  - **Encapsulation** – replace the C `struct json_value` with a `Value` class hierarchy (`ValueNull`, `ValueBoolean`, `ValueNumber`, `ValueString`, `ValueArray`, `ValueObject`).  
  - **SRP** – each concrete class handles a single JSON type.  
  - **Composition** – `ValueObject` composes a `HashTable` for member storage; `ValueArray` composes a `StrBuffer`‑backed dynamic array.  
  - **RAII** – each derived class manages its own child values via smart pointers.  
- **Modern C++ changes**:
  - Use `std::variant` or a custom hierarchy instead of a union.  
  - Replace raw `char*` strings with `std::string_view` (or `std::string` owned by `ValueString`).  
  - Use `std::unique_ptr<Value>` for child ownership.  
  - Apply `constexpr` where possible for static type information.  
- **Public API outcome**: `class Value { public: enum class Type { Null, Boolean, Number, String, Array, Object };`; … }` with factory functions `Value MakeNull(); Value MakeObject();` etc.  
- **Tests/verification**: Ensure that serialization/deserialization behavior matches the original C implementation, especially edge cases like empty objects/arrays.

---

## Phase 5 – I/O and Dump/Load Modules

### Step 9 – Convert `load.c` → `cpp/load.cpp`
- **Purpose**: Reads JSON from a file or file descriptor into a `Value` tree.  
- **Internal deps**: `error.c`, `utf.c`, `strbuffer.c`, `value.c`.  
- **OOP principles to apply**:
  - **Abstraction** – introduce an `IReader` interface; concrete `FileReader` implements it.  
  - **SRP** – `JsonLoader` orchestrates reading but delegates I/O to `IReader`.  
  - **RAII** – use `std::ifstream` with RAII; no manual file handle leaks.  
- **Modern C++ changes**:
  - Replace `FILE*` usage with `std::ifstream`.  
  - Use `std::string_view` for chunked reading; avoid manual buffer management.  
  - Throw `JanssonException` on I/O errors instead of returning error codes.  
- **Public API outcome**: `class JsonLoader { public: static std::unique_ptr<Value> LoadFromPath(const std::string&); };`  
- **Tests/verification**: Compare parsed AST structure against reference outputs from the original C `load.c`.

### Step 10 – Convert `dump.c` → `cpp/dump.cpp`
- **Purpose**: Serializes a `Value` tree back to a JSON string (used for output and testing).  
- **Internal deps**: `value.c`, `strbuffer.c`, `error.c`.  
- **OOP principles to apply**:
  - **Encapsulation** – move dumping logic into a `JsonDumper` class that receives a `const Value&`.  
  - **SRP** – each dumping mode (object, array, number) is a separate private method.  
  - **Composition** – reuse `StrBuffer` for efficient string building.  
- **Modern C++ changes**:
  - Replace manual buffer handling with `StrBuffer` class (already converted).  
  - Use `std::string_view` for key strings; avoid raw `char*`.  
  - Provide overloads that accept `std::ostream&` for flexible output.  
- **Public API outcome**: `class JsonDumper { public: static std::string Dump(const Value&, std::string_view indent = "  "); };`  
- **Tests/verification**: Ensure the generated JSON string is byte‑identical to the original C output for a set of sample values.

---

## Phase 6 – Miscellaneous Utilities

### Step 11 – Convert `pack_unpack.c` → `cpp/pack_unpack.cpp`
- **Purpose**: Implements packing/unpacking of JSON values to/from binary representations (used for storage engines).  
- **Internal deps**: `value.c`, `error.c`.  
- **OOP principles to apply**:
  - **Encapsulation** – wrap functionality in a `PackUnpack` namespace with static methods.  
  - **SRP** – separate packing and unpacking into distinct methods.  
  - **RAII** – use `std::vector<uint8_t>` for dynamic buffers.  
- **Modern C++ changes**:
  - Replace C arrays with `std::vector<uint8_t>`.  
  - Use `std::optional` for partially unpacked results.  
  - Prefer `reinterpret_cast`‑free code; use `std::memcpy_s` where required.  
- **Public API outcome**: `namespace PackUnpack { static std::vector<uint8_t> Pack(const Value&); static std::optional<Value> Unpack(std::string_view); };`  
- **Tests/verification**: Validate that packed binary matches the original format and that unpacking reproduces equivalent `Value` objects.

### Step 12 – Convert `dtoa.c` → `cpp/dtoa.cpp`
- **Purpose**: Double‑to‑string conversion utilities used by number serialization.  
- **Internal deps**: `error.c`, `math.h`.  
- **OOP principles to apply**:
  - **Encapsulation** – expose `DoubleToString` as a static method of a `Dtoa` helper class.  
  - **SRP** – each rounding mode is a separate overload.  
  - **RAII** – use `std::string` for output; no manual buffer allocation.  
- **Modern C++ changes**:
  - Replace custom `strtod`/`dopr` logic with `std::to_chars` (C++17) for deterministic, locale‑independent conversion.  
  - Return `std::string` directly; avoid returning raw pointers.  
- **Public API outcome**: `class Dtoa { public: static std::string DoubleToString(double, int precision = 6); };`  
- **Tests/verification**: Compare converted strings against reference outputs from the original C version.

---

## Phase 7 – Memory Management Helper

### Step 13 – Convert `memory.c` → `cpp/memory.cpp`
- **Purpose**: Provides low‑level memory allocation/free helpers used throughout the library.  
- **Internal deps**: `error.c` (error reporting).  
- **OOP principles to apply**:
  - **RAII** – eliminate manual `malloc`/`free`; replace with `std::unique_ptr` and `std::vector` for dynamic storage.  
  - **Encapsulation** – expose a `Memory` namespace with `Allocate<T>(size_t)` returning `std::unique_ptr<T[]>`.  
  - **SRP** – each allocation strategy (stack, heap) is a separate function.  
- **Modern C++ changes**:
  - Remove all `void*`‑based allocation; use type‑safe `new[]` wrapped in smart pointers.  
  - Replace custom allocator with `std::pmr::monotonic_buffer_resource` if needed (optional).  
- **Public API outcome**: `namespace Memory { template<typename T> std::unique_ptr<T[]> Allocate(size_t n); }`  
- **Tests/verification**: Ensure that allocation sizes and alignment behavior remain identical to the original C implementation.

---

### Final Remarks
* **Testing Strategy** – After each step, run the existing test suite (or the provided validation harness) to confirm behavioral equivalence.  
* **Incremental Integration** – Once a phase is completed, integrate the newly converted modules with the already‑converted dependencies before proceeding to the next phase.  
* **Documentation** – Update any public headers to reflect the new C++ signatures; keep the old C headers as compatibility wrappers if needed.  

*This plan provides a concrete, step‑by‑step roadmap that respects dependency order, applies modern C++17 best practices, and explicitly calls out the OOP principles enforced at each conversion stage.*  

