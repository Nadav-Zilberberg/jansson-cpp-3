# Conversion Plan (C -> C++17)

## Overview
This document translates the high‑level architectural direction in `plan_overview.md` into a concrete, phased execution plan. Following the steps below will result in a clean separation between legacy C code (`c_src/`) and modern C++ code (`src/`), with all C++ sources compiled using C++17 and adhering to the principles outlined in `plan_overview.md`.

## Files to Exclude
The following source files are **not** converted to C++. They are low‑level utility implementations that are either already portable C, provide no API surface, or are better left untouched to avoid unnecessary churn.

| File | Reason for Exclusion |
|------|----------------------|
| `c_src/strbuffer.c` | Implements a simple string buffer used only internally; no public API. |
| `c_src/hashtable.c` | Stand‑alone hash table implementation used by the parser; no external dependencies. |
| `c_src/memory.c` | Provides a thin wrapper around `malloc`/`free`; can be replaced by `std::malloc`/`std::free` or removed. |
| `c_src/version.c` | Returns a static version string; trivial to keep as C. |
| `c_src/utf.c` | Utility functions for UTF‑8 handling; pure C and has no C++‑specific state. |
| `c_src/pack_unpack.c` | Binary packing/unpacking helpers; used only by internal parsers. |

These files remain in `c_src/` to preserve legacy functionality and to avoid converting code that does not affect the public interface.

## Phase 0: Setup
- **Goal**: Prepare the project for C++ compilation and create the foundational directory structure.
- **Steps**:
  1. Create the `src/` directory at the repository root.
  2. Add a new `CMakeLists.txt` (or modify the existing one) to:
     - Set the C++ standard to 17 (`set(CMAKE_CXX_STANDARD 17)`).
     - Add the `src/` directory as a library (`add_library(jsoncpp_lib src/*.cpp)`).
     - Keep the existing `c_src/` directory untouched for legacy builds.
  3. Add a `Makefile` target that builds both the legacy C library and the new C++ library.
  4. Verify that `cmake .` completes without errors and that `make` builds the new library.
- **Entry Condition**: `src/` directory exists and CMake configures a C++17 target.
- **Exit Condition**: `src/` is ready to receive C++ source files; build system compiles a dummy `main.cpp` (to be added later).

## Phase 1: Foundational Structure and Core Dependencies
*Focus: Create core C++ infrastructure that will be reused by all later components.*

### Step 1: Create `src/json_value.hpp` and `src/json_value.cpp`
- **Purpose**: Define the `JsonValue` class representing a JSON value (object, array, string, number, boolean, null).
- **Dependencies**: None (Phase 1 is leaf‑free).
- **OOP Principles**: Encapsulation, RAII (no manual memory management inside the class).
- **Modern C++ Changes**:
  - Use `std::unique_ptr` for object/array children.
  - Use `std::string` for string values.
  - Use `double` for numbers.
- **Verification**: Compile and run a simple test that constructs a `JsonValue` and prints its type.

### Step 2: Create `src/json_parser.hpp` and `src/json_parser.cpp`
- **Purpose**: Implement a recursive‑descent parser that builds `JsonValue` trees from a C‑string input.
- **Dependencies**: Relies on `json_value.hpp`.
- **OOP Principles**: Single Responsibility (parsing only), Encapsulation (parser state hidden).
- **Modern C++ Changes**:
  - Use `std::string_view` for input scanning.
  - Throw `std::runtime_error` on syntax errors.
- **Verification**: Parse a small JSON snippet and verify the resulting tree structure.

### Step 3: Create `src/json_serializer.hpp` and `src/json_serializer.cpp`
- **Purpose**: Convert a `JsonValue` back to a JSON string.
- **Dependencies**: Relies on `json_value.hpp`.
- **OOP Principles**: Separation of concerns (serialization distinct from parsing).
- **Modern C++ Changes**:
  - Use `std::ostringstream` for output building.
  - Provide overloaded `serialize` methods for each `JsonValue` type.
- **Verification**: Serialize the tree from Step 2 and compare to the original input.

### Step 4: Create `src/json_api.hpp`
- **Purpose**: Export a C‑compatible façade that mirrors the original C API but forwards to the C++ implementation.
- **Dependencies**: None.
- **OOP Principles**: Interface segregation (only expose necessary functions).
- **Modern C++ Changes**:
  - Use `extern "C"` for functions that must remain C‑linkage.
  - Wrap C++ objects in `std::unique_ptr` and manage lifetimes via opaque pointers.
- **Verification**: Compile a small C test file that calls the façade and confirm behavior matches the original C library.

## Phase 2: Core and Mid‑Level Components
*Focus: Convert the primary logic components that depend on Phase 1 foundations.*

### Step 5: Convert `c_src/value.c` → `src/value.cpp`
- **Purpose**: Replace the C implementation of the JSON value structure with the C++ `JsonValue` class.
- **Dependencies**: None beyond Phase 1 files.
- **OOP Principles**: Value semantics, proper copy/move semantics.
- **Modern C++ Changes**: Remove manual reference counting; rely on `std::unique_ptr`.

### Step 6: Convert `c_src/load.c` → `src/loader.cpp`
- **Purpose**: Load JSON data from a file or memory buffer into the C++ parser.
- **Dependencies**: Uses `json_parser.hpp`.
- **Modern C++ Changes**: Use `std::filesystem` for file handling; return `std::unique_ptr<JsonValue>`.

### Step 7: Convert `c_src/dump.c` → `src/dumper.cpp`
- **Purpose**: Serialize a `JsonValue` to a file or output stream.
- **Dependencies**: Uses `json_serializer.hpp`.
- **Modern C++ Changes**: Use `std::ofstream` and `std::ostringstream`.

### Step 8: Convert `c_src/error.c` → `src/error.cpp`
- **Purpose**: Provide C++ wrappers for error handling that map to C++ exceptions.
- **Dependencies**: None.
- **Modern C++ Changes**: Replace `jansson_error` struct with `std::string` messages; throw exceptions on error conditions.

### Step 9: Convert `c_src/memory.c` → `src/memory_manager.hpp`
- **Purpose**: Abstract memory allocation strategies; keep as a utility header only.
- **Dependencies**: None.
- **Modern C++ Changes**: Use `std::allocator`; no manual `malloc`/`free`.

## Phase 3: Peripheral and Leaf Components
*Focus: Convert low‑dependency or edge components, finalize integration.*

### Step 10: Convert `c_src/hashtable.c` → `src/hashed_string.hpp`
- **Purpose**: Provide a hash function for string keys used by internal containers.
- **Dependencies**: None.
- **Modern C++ Changes**: Use `std::hash<std::string>`; no custom hash table needed.

### Step 11: Convert `c_src/strconv.c` → `src/string_converter.hpp`
- **Purpose**: Convert between C strings and `std::string`/`std::string_view`.
- **Dependencies**: None.
- **Modern C++ Changes**: Use `std::stoi`, `std::stod` for numeric conversions.

### Step 12: Convert `c_src/dtoa.c` → `src/dtoa.hpp`
- **Purpose**: Convert doubles to string representations.
- **Dependencies**: None.
- **Modern C++ Changes**: Use `std::to_string` or `std::ostringstream`.

### Step 13: Convert `c_src/utf.c` → `src/utf8_validator.hpp`
- **Purpose**: Validate UTF‑8 sequences.
- **Dependencies**: None.
- **Modern C++ Changes**: Use `std::string_view` and range‑based checks.

### Step 14: Convert `c_src/pack_unpack.c` → `src/pack_unpack.hpp`
- **Purpose**: Provide utilities for binary packing/unpacking used by the parser.
- **Dependencies**: None.
- **Modern C++ Changes**: Use `std::vector<uint8_t>` and `std::bit_cast`.

## Phase 4: Build System and Final Validation
- **Goal**: Ensure the legacy C build and the new C++ build coexist, then retire the C build.
- **Steps**:
  1. Add a `tests/` directory with existing unit tests (already handled per instructions).
  2. Update CI configuration to compile the C++ library with `-Wall -Wextra -pedantic`.
  3. Run the full test suite; verify that all tests pass against the new C++ implementation.
  4. Remove any remaining `#include "c_src/*.c"` references from the build system.
  5. Tag the repository as `conversion-complete`.

## C-to-C++ Conversion Rules and Patterns
- **Struct → Class**: Replace C `struct` with a C++ class; make data members private; provide public getters/setters as needed.
- **Function → Method**: Move free functions into the appropriate class; use `this->` for member access.
- **Ownership**: Transfer ownership via `std::unique_ptr`; avoid raw `new`/`delete`.
- **Strings**: Use `std::string` for mutable text, `std::string_view` for read‑only slices.
- **Error Handling**: Replace C error codes with `std::runtime_error` exceptions.
- **Memory**: Eliminate manual `malloc`/`free`; use RAII containers (`std::vector`, `std::unique_ptr`).

## Class Design and Implementation Details
| Class | Responsibility | Key Methods | Important Members |
|-------|----------------|-------------|-------------------|
| `JsonValue` | Represents any JSON value | `type()`, `asObject()`, `asArray()`, `asString()` | `std::unique_ptr<JsonObject> object_;`, `std::unique_ptr<JsonArray> array_;`, `std::string string_;`, `double number_;`, `bool boolean_;`, `std::monostate null_;` |
| `JsonObject` | Holds key‑value pairs | `add(key, value)`, `get(key)` | `std::unordered_map<std::string, JsonValue> items_;` |
| `JsonArray` | Ordered list of values | `add(value)`, `operator[](index)` | `std::vector<JsonValue> items_;` |
| `JsonParser` | Parses text into `JsonValue` | `parse(const std::string_view&)` | `std::unique_ptr<JsonValue> root_;` |
| `JsonSerializer` | Serializes `JsonValue` to text | `serialize(const JsonValue&)` | (none) |
| `JsonAPI` | C‑compatible façade | `json_create()`, `json_parse(const char*)`, `json_serialize(const JsonValue*)` | Internal wrappers holding `std::unique_ptr<JsonParser>` and `std::unique_ptr<JsonSerializer>` |

## Project‑Wide and Non‑Code Changes
- **Directory Layout**: All new C++ source files reside under `src/`; legacy C files remain under `c_src/`.
- **Compiler Flags**: Enable `-std=c++17 -Wall -Wextra -pedantic` for all C++ compilation units.
- **Linkage**: The new library is built as a shared object (`libjsoncpp_lib.so`) alongside the legacy static library (`libjsonc.a`).
- **Header Guards**: All C++ headers must use `#pragma once` or traditional include guards.
- **Naming Conventions**: Use `camelCase` for public methods, `UPPER_SNAKE` for macros/constants.
- **Documentation**: Doxygen comments must be added to all public classes and methods.

## Final Validation and Completion Checklist
- [ ] All files listed in **Files to Exclude** remain untouched in `c_src/`.
- [ ] The `src/` directory contains only C++ source files (`*.cpp`, `*.hpp`).
- [ ] No `#include` directives reference `c_src/*.c` from C++ code.
- [ ] The build system compiles a test program that uses the C++ façade and produces identical output to the original C version.
- [ ] All unit tests pass against the new C++ implementation.
- [ ] The repository builds both the legacy C library and the new C++ library without errors.
- [ ] The `plan.md` document itself follows the exact template structure provided in the ticket.