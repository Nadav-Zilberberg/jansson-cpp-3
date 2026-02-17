cmake_minimum_required(VERSION 3.10)
project(JanssonCpp VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(src)

# Source files
set(SOURCES
    src/json_error.cpp
    src/string_utils.cpp
    src/json_value.cpp
    src/json_parser.cpp
    src/json_serializer.cpp
    src/json_c_api.cpp
)

# Create library
add_library(jansson_cpp STATIC ${SOURCES})

target_include_directories(jansson_cpp PUBLIC src)

# Create example executable
add_executable(example example.cpp)
target_link_libraries(example jansson_cpp)

# Install targets
install(TARGETS jansson_cpp
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(TARGETS example
        RUNTIME DESTINATION bin)

install(FILES src/json_error.hpp
              src/string_utils.hpp
              src/memory_policy.hpp
              src/json_hash.hpp
              src/json_value.hpp
              src/json_parser.hpp
              src/json_serializer.hpp
              src/json_c_api.hpp
        DESTINATION include/jansson)
