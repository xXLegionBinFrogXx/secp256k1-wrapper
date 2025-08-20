# secp256k1-wrapper

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)
[![C Standard](https://img.shields.io/badge/C-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)

A modern, lightweight C wrapper library for [libsecp256k1](https://github.com/bitcoin-core/secp256k1) that simplifies elliptic curve cryptography operations for secp256k1 keys.

## Features

✅ **Simple API** - Easy-to-use functions for key generation and derivation  
✅ **Cross-Platform** - Windows, macOS, Linux, and BSD support  
✅ **Secure** - Platform-specific secure random number generation  
✅ **Thread-Safe** - No global state, context management per operation  
✅ **Modern Build** - CMake-based build system using latest secp256k1 v0.7.0 features  
✅ **Well Tested** - Comprehensive test suite with Unity framework  
✅ **Zero Dependencies** - No git submodules or vendored code, all dependencies auto-fetched  

## Quick Start

### Clone Repository
```bash
git clone https://github.com/xXLegionBinFrogXx/secp256k1-wrapper.git
cd secp256k1-wrapper
```

### Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Run Tests
```bash
cmake --build . --target test_wrapper
ctest
```

### Run Example
```bash
./demo                 # 2 key pairs, compressed pubkeys (default)
./demo 5               # 5 key pairs, compressed
./demo 3 u             # 3 key pairs, uncompressed
./demo 3 compressed    # explicit compressed format
```

## API Overview

```c
#include <secp256k1_wrapper.h>

// Generate a new key pair
unsigned char privkey[32];
unsigned char pubkey[33];  // 33 bytes for compressed, 65 for uncompressed
int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 1); // 1 = compressed

// Derive public key from existing private key
unsigned char derived_pubkey[33];
int result = secp256k1_wrapper_derive_pubkey(privkey, derived_pubkey, 1);

// Get library version
const char* version = secp256k1_wrapper_get_version();

// Generate secure random bytes
unsigned char random_data[32];
int success = secp256k1_wrapper_fill_random(random_data, 32);
```

## Build Options

```bash
# Full build with all options (default)
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_TESTS=ON \
         -DBUILD_EXAMPLES=ON \
         -DBUILD_SHARED=ON \
         -DBUILD_STATIC=ON

# Minimal build - static library only
cmake .. -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_SHARED=OFF

# Debug build with full logging
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Installation

```bash
# Install to system directories (requires sudo on Unix)
cmake --build . --target install

# Install to custom prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/secp256k1-wrapper
cmake --build . --target install
```

## Library Outputs

- **`libsecp256k1-wrapper.a`** - Static library
- **`libsecp256k1-wrapper.so`** - Shared library (Linux)
- **`libsecp256k1-wrapper.dylib`** - Dynamic library (macOS)
- **`secp256k1-wrapper.dll`** - Dynamic library (Windows)

## Platform Support

| Platform | Random Source | Notes |
|----------|--------------|-------|
| **Windows** | `BCryptGenRandom` | Uses Windows Crypto API |
| **Linux** | `getrandom()` syscall | Falls back to `/dev/urandom` |
| **macOS** | `CCRandomGenerateBytes` | Falls back to `getentropy()` |
| **OpenBSD** | `getentropy()` | Limited to 256 bytes per call |
| **FreeBSD** | `getrandom()` syscall | Similar to Linux |

## Dependencies

- **CMake 3.15+** - Build system
- **C99 compiler** - GCC, Clang, or MSVC
- **libsecp256k1 v0.7.0+** - Automatically fetched via CMake FetchContent  
- **Unity v2.6.0** - Test framework (automatically fetched when BUILD_TESTS=ON)

All dependencies are automatically downloaded and built - no git submodules or manual dependency management required.

## Security Features

- **Context randomization** - Protection against side-channel attacks
- **Secure memory clearing** - Sensitive data is properly zeroed
- **Private key validation** - All generated keys are verified
- **No global state** - Thread-safe design


## Error Codes

| Code | Description |
|------|-------------|
| `0` | Success |
| `-1` | Invalid input parameters |
| `-2` | Context creation/randomization failed |
| `-3` | Random number generation failed |
| `-5` | Public key creation/serialization failed |

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

This project incorporates code from [libsecp256k1](https://github.com/bitcoin-core/secp256k1), which is also MIT licensed.

## Acknowledgments

- **Bitcoin Core Developers** - For the excellent libsecp256k1 library
- **ThrowTheSwitch** - For the Unity testing framework
