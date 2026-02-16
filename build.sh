#!/bin/bash

echo "Building Jansson C++ JSON Library..."

# Create build directory
mkdir -p build
cd build

# Run CMake
cmake ..

# Build the project
make

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful!"
    
    # Run the example
    echo "Running example..."
    ./example
else
    echo "Build failed!"
    exit 1
fi
