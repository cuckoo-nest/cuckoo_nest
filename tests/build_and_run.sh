#!/bin/bash

# Build and run unit tests for Cuckoo Nest project

set -e  # Exit on any error

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/build-tests"

echo "Building Cuckoo Nest Unit Tests..."
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"

# Create and enter build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with tests enabled
echo "Configuring build..."
cmake -DBUILD_TESTS=ON "$PROJECT_ROOT"

# Build
echo "Building tests..."
make -j$(nproc)

# Run tests
echo "Running tests..."
echo "================================"
./tests/cuckoo_tests

echo ""
echo "All tests completed!"