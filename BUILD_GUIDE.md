# Build Guide & Performance Tips

## Quick Start (Fast Development Builds with Code Quality)

```bash
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/host-toolchain.cmake ..
cmake --build . -j$(nproc)
```

**Default behavior:** Clang-tidy is **enabled** for code quality checks. With ccache, builds are still fast.

## Building without Static Analysis (Fastest)

To skip clang-tidy checks for maximum speed:

```bash
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/host-toolchain.cmake -DENABLE_CLANG_TIDY=OFF ..
cmake --build . -j$(nproc)
```

## Building with Static Analysis (Default)

Clang-tidy runs by default with ccache for efficient caching:

## Clean Rebuild

```bash
rm -rf build
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/host-toolchain.cmake ..
cmake --build .
```

## Performance Optimization Tips

### 1. ccache (Automatic)

ccache is **auto-detected and enabled** during CMake configuration if installed:

```bash
# Install ccache (if not already installed):
# Ubuntu/Debian
sudo apt-get install ccache

# macOS
brew install ccache
```

Once installed, just run `cmake ..` and it will be automatically enabled. Subsequent builds will be dramatically faster (~10x).

### 2. Parallel Build Jobs

Build with multiple cores:
```bash
cmake --build . -j$(nproc)  # Uses all available cores
cmake --build . -j8          # Or specify explicitly
```

### 3. Run Tests

```bash
ctest --output-on-failure -j$(nproc)
```

## Recommended Workflow

### Standard Development (with code quality):
```bash
cd build
cmake --build . -j$(nproc)  # Clang-tidy enabled by default
```

### Maximum Speed (skip linting temporarily):
```bash
cd build
cmake -DENABLE_CLANG_TIDY=OFF ..
cmake --build . -j$(nproc)  # No linting, fastest possible
```

### CI/CD:
Clang-tidy is enabled by default in GitHub Actions for all builds.

## Options

### Disable clang-tidy:
```bash
cmake -DENABLE_CLANG_TIDY=OFF ..
```

### Legacy skip option:
```bash
cmake -DSKIP_CLANG_TIDY=ON ..
```

## Troubleshooting

**"clang-tidy not found"**
```bash
# Install clang-tools
sudo apt-get install clang-tools
```

**Build cache issues**
```bash
# Clear CMake cache
rm -rf build/CMakeCache.txt build/CMakeFiles
cmake ..
```

**Out of memory during build**
- Reduce parallel jobs: `cmake --build . -j4`
- Enable swap if available

## Build Time Reference (with ccache)

| Configuration | Time (Estimated) |
|---|---|
| Full clean build (with clang-tidy, ccache empty) | 8-12 min |
| Full clean build (no clang-tidy, ccache empty) | 2-3 min |
| Incremental build (with clang-tidy, ccache hit) | 5-15 sec |
| Incremental build (no clang-tidy, ccache hit) | 2-5 sec |

