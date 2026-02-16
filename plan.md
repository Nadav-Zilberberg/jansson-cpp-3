# Conversion Plan (C -> C++17)
## Overview
- The conversion is split into phases to respect dependency order, starting with self‑contained modules and ending with the public API.
- Phase boundaries are defined by the set of other C files a component references; a phase is complete when all files it depends on have been migrated or replaced.
- Early phases create the build infrastructure and the first C++ classes that will be reused by later phases.

## files to exclude
- strconv.c – implements low‑level string conversion utilities that are already provided by std::to_string and std::stringstream; converting it would add no functional value.
- hashtable.c – the original hash table is a raw open‑addressing implementation; the C++ version will be replaced by std::unordered_map, making the C file unnecessary.
- pack_unpack.c – performs binary packing of JSON numbers; std::bitset and bit‑wise operators can replace its functionality without loss of precision.
- dump.c – contains a simple pretty‑printer; the C++ serializer will generate formatted output using std::ostringstream and indentation logic, so the C file can be omitted.
- memory.c – wraps malloc/free; the C++ standard library provides RAII‑based allocators, so this file adds redundancy.
- strbuffer.c – manages a growing character buffer; std::string with push_back offers a safer, automatically resizing alternative.
- These files are listed because they either duplicate functionality that the standard library already supplies or because their logic will be superseded by modern C++ components introduced in later phases.

## Setup
- Create the top‑level src/ directory; this will hold all new C++ source files.
- Add src/ to the CMakeLists.txt include directories and to the library target’s source list.
- Edit the top‑level Makefile.am (or configure.ac) to recognize the new directory and to exclude c_src/ from the final link step once migration is complete.
- Example change in CMakeLists.txt:
  add_library(jansson_lib src/jansson.cpp src/jansson/value.cpp ... )
  target_include_directories(jansson_lib PUBLIC src)
- These modifications are required because the build system must know where to find the new C++ files and must stop pulling in legacy C objects after the migration is finished.

## Phase 1: Leaf modules (no internal deps)
Step 1: Create version.hpp and version.cpp in src/jansson/
- Purpose: Provide a thin C++ wrapper around the existing version.c API.
- Deps: None – version.c has no other C files it includes.
- OOP to apply: Encapsulation – hide the raw version string behind a class.
- Modern C++ changes: Use std::string for the version text, constexpr for compile‑time constants, and final for class sealing.
- Verification: Build the module and run the existing version test; the C++ wrapper must return the same string.

Step 2: Create error.hpp and error.cpp in src/jansson/
- Purpose: Wrap error.c’s error code handling with an exception‑based interface.
- Deps: None.
- OOP to apply: SRP – a single class that maps error codes to descriptive messages.
- Modern C++ changes: Use std::runtime_error for error throwing, and a static lookup table with enum class.
- Verification: Compile and run the error test suite; error messages must match the original C output.

Step 3: Create memory.hpp and memory.cpp in src/jansson/
- Purpose: Replace memory.c’s malloc/free wrappers with std::unique_ptr and std::make_unique.
- Deps: None.
- OOP to apply: RAII – allocate memory via smart pointers, eliminating manual free calls.
- Modern C++ changes: Use std::unique_ptr for owned buffers, and provide factory functions returning std::unique_ptr<void[]>.
- Verification: Run the memory‑leak test (valgrind) to ensure no leaks remain.

Step 4: Create strbuffer.hpp and strbuffer.cpp in src/jansson/
- Purpose: Provide a C++ version of strbuffer.c that uses std::string for dynamic buffering.
- Deps: None.
- OOP to apply: Encapsulation – expose only append and str methods.
- Modern C++ changes: Use std::string::reserve for capacity hinting, and move semantics for final string extraction.
- Verification: Ensure that the output length and content are identical to the original C implementation.

## Phase 2: Next dependency level
Step 5: Create jsonvalue.hpp and jsonvalue.cpp in src/jansson/
- Purpose: Replace value.c with a class that models a JSON value using std::variant for type safety.
- Deps: version.hpp, error.hpp, memory.hpp, strbuffer.hpp (all created in Phase 1).
- OOP to apply: Encapsulation and SRP – the class owns its children and provides methods for object, array, string, number, boolean, and null.
- Modern C++ changes: Use std::variant<std::monostate, bool, int, double, std::string, Object, Array> for the value type; use std::unique_ptr for object and array storage.
- Verification: Run the full unit test suite for value operations; all type‑specific tests must pass.

Step 6: Create jsonobject.hpp and jsonobject.cpp in src/jansson/
- Purpose: Implement object storage that replaces hashtable.c’s internal table.
- Deps: jsonvalue.hpp (from Step 5) and error.hpp.
- OOP to apply: Encapsulation – hide the underlying std::unordered_map<std::string, JsonValue> behind a class.
- Modern C++ changes: Use std::unordered_map with std::string keys and JsonValue values; provide iterator methods that return JsonObject::iterator references.
- Verification: Verify that object insertion, lookup, and deletion behave identically to the C hashtable.

## Phase 3: Core library modules
Step 7: Create json.hpp and json.cpp in src/jansson/
- Purpose: Consolidate the public C++ API (parser, serializer, value manipulation) into a single header/cpp pair.
- Deps: jsonvalue.hpp, jsonobject.hpp, error.hpp, memory.hpp.
- OOP to apply: API Layer – expose only the intended public methods; hide internal helpers.
- Modern C++ changes: Use explicit constructors, delete copy semantics where move is preferred, and annotate with [[nodiscard]] where appropriate.
- Verification: Build the library and run the integration test that parses and serializes a sample JSON document; output must be byte‑identical to the C version.

Step 8: Create parser.hpp and parser.cpp in src/jansson/
- Purpose: Replace load.c with a C++ parser that returns JsonValue objects.
- Deps: jsonvalue.hpp, error.hpp, memory.hpp.
- OOP to apply: SRP – the parser only knows how to read tokens and construct JsonValue trees.
- Modern C++ changes: Use std::unique_ptr<JsonValue> for parsed trees, and employ std::string_view for token slicing.
- Verification: Run the parser test suite; the AST structure must match the reference implementation.

Step 9: Create serializer.hpp and serializer.cpp in src/jansson/
- Purpose: Replace dump.c with a serializer that outputs JSON from JsonValue objects.
- Deps: jsonvalue.hpp, jsonobject.hpp, error.hpp.
- OOP to apply: Encapsulation – the serializer only writes to an output stream.
- Modern C++ changes: Use std::ostringstream with manual indentation; avoid manual buffer management.
- Verification: Compare generated JSON strings against golden files; they must be identical.

## Phase 4: API layer and integration
Step 10: Create jansson.hpp in src/jansson/
- Purpose: Provide a C‑compatible header that forwards to the new C++ implementation, preserving the original function signatures.
- Deps: All previously created C++ files.
- OOP to apply: API Layer – keep a thin procedural façade for backward compatibility.
- Modern C++ changes: Use extern "C" wrappers around C++ classes; ensure name mangling does not affect the C API.
- Verification: Compile the existing examples against the new library; they must link and behave exactly as before.

Step 11: Update the public Makefile target to link against src/ objects instead of c_src/ objects.
- Purpose: Transition the build to use only the new C++ library.
- Deps: Successful completion of Steps 1‑10.
- OOP to apply: None – this is a build‑system change.
- Modern C++ changes: Remove -Ic_src from include paths, add -Isrc.
- Verification: Run the full test suite; all tests must pass and the library version must report the same version string as before.

## C-to-C++ Conversion Rules and Patterns
- Struct-to-class mapping: Any C struct that owns data becomes a class with private members and public accessor methods. Example: struct json_value becomes class JsonValue.
- Function-to-method transformation: Free functions that operate on a specific struct are moved into that struct’s class as member functions. Example: json_value_parse becomes JsonValue::parse.
- Ownership and lifetime: Replace manual malloc/free with std::unique_ptr; replace raw pointers to owned objects with std::shared_ptr only when multiple owners are required.
- Use of standard library types: Replace char* buffers with std::string, replace C arrays with std::vector, replace C enums with enum class.
- Error handling: Convert return‑code checks into exception throws or into std::error_code returns, depending on the desired error‑propagation model.
- Const‑correctness: Mark methods that do not modify state with const; use constexpr where possible for compile‑time constants.
- RAII: Acquire resources (memory, file handles) in constructors and release them in destructors; avoid explicit free calls outside of destructors.
- Move semantics: Where a function takes ownership of a pointer, change the parameter to std::unique_ptr&& or T&& to enable move‑only semantics.
- Template usage: When a function is generic over a type (e.g., a hash function), replace the C qsort-based approach with std::sort and a lambda comparator.
