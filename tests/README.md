# Cuckoo Nest Unit Tests

This directory contains unit tests for the Cuckoo Nest project using Google Test framework.

## Building and Running Tests

### Prerequisites
- CMake 3.20 or higher
- A C++11 compatible compiler (gcc/clang)
- Internet connection (for downloading Google Test)

### Building Tests

From the project root directory:

```bash
# Create a separate build directory for tests
mkdir build-tests
cd build-tests

# Configure the build with tests enabled
cmake -DBUILD_TESTS=ON ..

# Build the tests
make -j$(nproc)
```

### Running Tests

After building, run the tests:

```bash
# Run all tests
./cuckoo_tests

# Run tests with verbose output
./cuckoo_tests --gtest_verbose

# Run specific test cases
./cuckoo_tests --gtest_filter="ScreenManagerTest.*"
```

### Running with CTest

You can also use CTest to run the tests:

```bash
# Run tests through CTest
ctest

# Run with verbose output
ctest --verbose

# Run specific tests
ctest -R ScreenManager
```

## Test Structure

- `test_screen_manager.cpp` - Tests for the ScreenManager class
- More test files can be added as needed

## Building Main Application

The main cross-compiled application can still be built separately:

```bash
# Create build directory for main app
mkdir build-main
cd build-main

# Configure for cross-compilation (default behavior)
cmake ..

# Build the main application
make -j$(nproc)
```

Or explicitly disable tests:

```bash
cmake -DBUILD_TESTS=OFF ..
```

## Test Coverage

The tests currently cover:
- ScreenManager instantiation
- Basic screen navigation functionality
- Error handling for edge cases

## Adding New Tests

1. Create new test files in the `tests/` directory
2. Add them to the `TEST_FILES` list in `tests/CMakeLists.txt`
3. If testing new source files, add them to the `TEST_SOURCE_FILES` list
4. Follow the Google Test framework conventions