# secp256k1-wrapper

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)
[![C Standard](https://img.shields.io/badge/C-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Build](https://github.com/xXLegionBinFrogXx/secp256k1-wrapper/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/xXLegionBinFrogXx/secp256k1-wrapper/actions/workflows/cmake-single-platform.yml)

A modern, lightweight C wrapper library for [libsecp256k1](https://github.com/bitcoin-core/secp256k1) that simplifies elliptic curve cryptography operations for secp256k1 keys.

## Features

| Feature                | Description                                                            |
| ---------------------- | ---------------------------------------------------------------------- |
| **Simple API**     | Easy-to-use functions for key generation and derivation                |
| **Cross-Platform**  | Supports Windows, macOS, Linux, and BSD                                |
| **Secure**          | Platform-specific secure random number generation                      |
| **Thread-Safe**     | No global state, context management per operation                      |
| **Modern Build**    | CMake-based build system using latest secp256k1 v0.7.0 features        |
| **Well Tested**      | Comprehensive test suite with Unity v2.6.1 framework                   |
| **No Manual Setup** | Dependencies auto-fetched via CMake, no git submodules or manual setup |

---

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
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON
cmake --build . --target test_wrapper   
ctest                                  
```

### Run Example

```bash
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build . --target demo

./demo                 # 2 key pairs, compressed pubkeys (default)
./demo 5               # 5 key pairs, compressed
./demo 3 u             # 3 key pairs, uncompressed
./demo 3 compressed    # explicit compressed format
```

---

## Installation

```bash
# Install to system directories (requires sudo on Unix)
cmake --build . --target install

# Install to custom prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/secp256k1-wrapper
cmake --build . --target install
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

## Library Outputs

* **`libsecp256k1-wrapper.a`** - Static library
* **`libsecp256k1-wrapper.so`** - Shared library (Linux)
* **`libsecp256k1-wrapper.dylib`** - Dynamic library (macOS)
* **`secp256k1-wrapper.dll`** - Dynamic library (Windows)

## Library Build Layout

This project produces two flavors of the wrapper:

1. **Shared wrapper (dynamic library)**

   * **Target**: `secp256k1-wrapper-shared` → installed as `libsecp256k1-wrapper.so` / `.dylib` / `.dll`.
   * **Upstream secp256k1** is linked statically into the wrapper.
     * You only need to link against `secp256k1-wrapper::secp256k1-wrapper-shared`.
     * At runtime, there is no dependency on an external `libsecp256k1.so`.
   * **Typical use case**: Provide a single `.so`/`.dll` that applications can link against, while we control the exact `secp256k1` version bundled inside.

2. **Static wrapper (static library)**

   * **Target**: `secp256k1-wrapper-static` → installed as `libsecp256k1-wrapper.a`.
   * **Upstream secp256k1** object files are embedded directly into this static archive.
     * You only need to link against `secp256k1-wrapper::secp256k1-wrapper-static`.
     * No external `secp256k1` library is required.
   * **Typical use case**: Fully static builds (CLI tools, embedded, mobile).

## Platform Support

| Platform    | Random Source           | Notes                         |
| ----------- | ----------------------- | ----------------------------- |
| **Windows** | `BCryptGenRandom`       | Uses Windows Crypto API       |
| **Linux**   | `getrandom()` syscall   | Falls back to `/dev/urandom`  |
| **macOS**   | `CCRandomGenerateBytes` | Falls back to `getentropy()`  |
| **OpenBSD** | `getentropy()`          | Limited to 256 bytes per call |
| **FreeBSD** | `getrandom()` syscall   | Similar to Linux              |

## Dependencies

* **CMake 3.24+** - Build system
* **C99 compiler** - GCC, Clang, or MSVC
* **libsecp256k1 v0.7.0+** - Automatically fetched via CMake FetchContent
* **Unity v2.6.1** - Test framework (auto-fetched when BUILD\_TESTS=ON)

All dependencies are automatically downloaded and built — no git submodules or manual dependency management required.

---

## Usage Examples

### C Usage

```c
#include <secp256k1_wrapper.h>
#include <stdio.h>

int main() {
    unsigned char privkey[32];
    unsigned char pubkey[33];  

    int result=secp256k1_wrapper_generate_keys(privkey, pubkey, 1);
    if (result != 0) {
        printf("Key generation failed: %d\n", result);
        return 1;
    }
    return 0;
}
```

### C++ Usage

```cpp
#include <secp256k1_wrapper.h>
#include <iostream>

int main() {
    unsigned char privkey[32];
    unsigned char pubkey[33];
    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 1);
    if (result != 0) {
        std::cerr<<"Key generation failed\n";
        return 1;
    }
    std::cout<<"Keys generated successfully\n";
}
```

### Go (CGO) Usage

```go
package main

/*
#cgo CFLAGS: -I/usr/local/include
#cgo LDFLAGS: -L/usr/local/lib -lsecp256k1-wrapper
#include <secp256k1_wrapper.h>
*/
import "C"
import "fmt"

func main() {
    var privkey [32]C.uchar
    var pubkey  [33]C.uchar
    result := C.secp256k1_wrapper_generate_keys(&privkey[0], &pubkey[0], 1)
    if result != 0 {
        fmt.Println("Key generation failed")
        return
    }
    fmt.Println("✅ Keys generated successfully")
}
```

---

## Error Codes

| Code | Description                              |
| ---- | ---------------------------------------- |
| `0`  | Success                                  |
| `-1` | Invalid input parameters                 |
| `-2` | Context creation/randomization failed    |
| `-3` | Random number generation failed          |
| `-5` | Public key creation/serialization failed |


## Security Features

* **Context randomization** — Protection against side-channel attacks
* **Secure memory clearing** — Sensitive data zeroed properly
* **Private key validation** — Keys always verified
* **No global state** — Thread-safe design

---

## Background & Resources

### For Cryptography Beginners

| Term                           | Description                                                                    |
| ------------------------------ | ------------------------------------------------------------------------------ |
| **secp256k1**                  | Elliptic curve used in Bitcoin                                                 |
| **Private Key**                | 32-byte secret used to sign transactions                                       |
| **Public Key**                 | Derived from the private key, 33 bytes (compressed) or 65 bytes (uncompressed) |
| **Key Derivation**             | Process of generating a public key from a private key                          |
| **Compressed vs Uncompressed** | Compressed saves space (33 vs 65 bytes)                                        |

### Learn More

* 📚 [Elliptic Curve Cryptography Guide](https://en.wikipedia.org/wiki/Elliptic_Curve_Digital_Signature_Algorithm)
* 📚 [Practical Cryptography for Developers](https://cryptobook.nakov.com/)
* 📚 [Introduction to secp256k1](https://en.bitcoin.it/wiki/Secp256k1)
* 📚 [libsecp256k1 GitHub](https://github.com/bitcoin-core/secp256k1)
* 📚 [Mastering Bitcoin](https://github.com/bitcoinbook/bitcoinbook)

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
It incorporates code from [libsecp256k1](https://github.com/bitcoin-core/secp256k1), also MIT licensed.

## Acknowledgments

* **Bitcoin Core Developers** — For libsecp256k1
* **ThrowTheSwitch** — For the Unity testing framework
