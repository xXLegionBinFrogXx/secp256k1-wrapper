# secp256k1-wrapper

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CMake](https://img.shields.io/badge/CMake-3.24+-blue.svg)](https://cmake.org/)
[![C Standard](https://img.shields.io/badge/C-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)
[![Build](https://github.com/xXLegionBinFrogXx/secp256k1-wrapper/actions/workflows/build.yml/badge.svg)](https://github.com/xXLegionBinFrogXx/secp256k1-wrapper/actions/workflows/build.yml)


A modern, lightweight C wrapper library for [libsecp256k1](https://github.com/bitcoin-core/secp256k1) that simplifies basic elliptic curve cryptography operations for secp256k1 keys.


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
cmake .. -DSECP256K1_WRAPPER_BUILD_TESTS=ON
cmake --build . --target test_wrapper   
ctest                                  
```

### Run Example

```bash
mkdir build && cd build
cmake .. -DSECP256K1_WRAPPER_BUILD_EXAMPLES=ON
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

> **Note:** this is a breaking change to the configure interface. `BUILD_TESTS` / `BUILD_EXAMPLES` /
> `BUILD_SHARED` / `BUILD_STATIC` no longer exist. Tests and examples are now
> `SECP256K1_WRAPPER_BUILD_TESTS` / `SECP256K1_WRAPPER_BUILD_EXAMPLES`, and library type is chosen with
> the standard `BUILD_SHARED_LIBS` — one library type is produced per configure directory.

```bash
# Full build: tests, examples, static library (default)
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DSECP256K1_WRAPPER_BUILD_TESTS=ON \
         -DSECP256K1_WRAPPER_BUILD_EXAMPLES=ON

# Shared library instead of static
cmake .. -DBUILD_SHARED_LIBS=ON

# Minimal build - static library only, no tests/examples (default)
cmake ..

# Debug build with full logging
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Library Outputs

* **`libsecp256k1-wrapper.a`** - Static library (default)
* **`libsecp256k1-wrapper.so`** - Shared library (Linux, with `-DBUILD_SHARED_LIBS=ON`)
* **`libsecp256k1-wrapper.dylib`** - Dynamic library (macOS, with `-DBUILD_SHARED_LIBS=ON`)

## Library Build Layout

One wrapper target is built per configure, chosen by `BUILD_SHARED_LIBS` (`OFF`/static by default).
**Upstream secp256k1** object files are embedded directly into the target either way — there is no
runtime or link-time dependency on a separate `libsecp256k1`.

Link against the target with either name:

* `secp256k1-wrapper` — always resolves to whichever type was built.
* `secp256k1-wrapper::static` or `secp256k1-wrapper::shared` — only the one matching the actual
  build exists, so linking the wrong one is a configure-time error rather than a surprise.

## Platform Support

**Linux is the supported and tested platform.** macOS is expected to work. Windows is not supported —
the build no longer links against `bcrypt`.

| Platform    | Random Source            | Notes                        |
| ----------- | ------------------------ | ----------------------------- |
| **Linux**   | `getrandom()` syscall    | Falls back to `/dev/urandom`  |
| **macOS**   | `CCRandomGenerateBytes`  | Falls back to `getentropy()`  |


## Dependencies

* **CMake 3.24+** - Build system
* **C99 compiler** - GCC or Clang
* **libsecp256k1 v0.8.0+** - Automatically fetched via CMake FetchContent
* **Unity v2.6.1** - Test framework (auto-fetched when SECP256K1\_WRAPPER\_BUILD\_TESTS=ON)

All dependencies are automatically downloaded and built — no git submodules or manual dependency management required.

---

## Usage Examples

### C Usage

```c
#include <secp256k1_wrapper.h>
#include <stdio.h>

int main() {
    unsigned char privkey[32];
    unsigned char pubkey[33];  // 33 bytes compressed

    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, sizeof(pubkey), 1);
    if (result != 0) {
        printf("Key generation failed: %d\n", result);
        return 1;
    }

    // Validate an externally-supplied private key without deriving a pubkey
    if (secp256k1_wrapper_verify_privkey(privkey) != 0) {
        printf("Private key is invalid\n");
    }

    // Securely wipe key material when done with it
    secp256k1_wrapper_memzero(privkey, sizeof(privkey));
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
    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, sizeof(pubkey), 1);
    if (result != 0) {
        std::cerr << "Key generation failed\n";
        return 1;
    }
    std::cout << "Keys generated successfully\n";
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
    var pubkey [33]C.uchar
    result := C.secp256k1_wrapper_generate_keys(&privkey[0], &pubkey[0], C.size_t(len(pubkey)), 1)
    if result != 0 {
        fmt.Println("Key generation failed")
        return
    }
    fmt.Println("Keys generated successfully")
}
```

---

## Error Codes

| Code | Description                                                  |
| ---- | -------------------------------------------------------------|
| `0`  | Success                                                       |
| `-1` | Invalid input parameters                                      |
| `-2` | Context creation failed                                       |
| `-3` | Random number generation failed                               |
| `-4` | Context randomization failed                                  |
| `-5` | Public key creation failed                                    |
| `-6` | Public key serialization failed                               |
| `-7` | Private key verification failed (`derive_pubkey` only)        |
| `-8` | Output buffer too small for the requested key format          |

## Error Handling Examples

### C Error Handling

```c
const char* get_error_message(int code) {
    switch (code) {
        case 0: return "Success";
        case -1: return "Invalid input parameters";
        case -2: return "Context creation failed";
        case -3: return "Random number generation failed";
        case -4: return "Context randomization failed";
        case -5: return "Public key creation failed";
        case -6: return "Public key serialization failed";
        case -7: return "Private key verification failed";
        case -8: return "Output buffer too small";
        default: return "Unknown error";
    }
}
```

### C++ Error Handling

```cpp
class Secp256k1Error : public std::runtime_error {
public:
    explicit Secp256k1Error(int code) 
        : std::runtime_error(getErrorMessage(code)), error_code_(code) {}
private:
    int error_code_;
    static std::string getErrorMessage(int code) {
        switch (code) {
            case -1: return "Invalid input parameters";
            case -2: return "Context creation failed";
            case -3: return "Random generation failed";
            case -4: return "Context randomization failed";
            case -5: return "Public key creation failed";
            case -6: return "Public key serialization failed";
            case -7: return "Private key verification failed";
            case -8: return "Output buffer too small";
            default: return "Unknown error";
        }
    }
};
```

### Go Error Handling

```go
func getErrorMessage(code int) error {
    switch code {
    case 0: return nil
    case -1: return errors.New("invalid input parameters")
    case -2: return errors.New("context creation failed")
    case -3: return errors.New("random number generation failed")
    case -4: return errors.New("context randomization failed")
    case -5: return errors.New("public key creation failed")
    case -6: return errors.New("public key serialization failed")
    case -7: return errors.New("private key verification failed")
    case -8: return errors.New("output buffer too small")
    default: return fmt.Errorf("unknown error (code: %d)", code)
    }
}
```

---

## Security Features

* **Context randomization** — Protection against side-channel attacks
* **Secure memory clearing** — Sensitive data zeroed properly; also exposed to callers as `secp256k1_wrapper_memzero()`
* **Private key validation** — Keys always verified; also exposed standalone as `secp256k1_wrapper_verify_privkey()`
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

* [Elliptic Curve Cryptography Guide](https://en.wikipedia.org/wiki/Elliptic_Curve_Digital_Signature_Algorithm)
* [Practical Cryptography for Developers](https://cryptobook.nakov.com/)
* [Introduction to secp256k1](https://en.bitcoin.it/wiki/Secp256k1)
* [libsecp256k1 GitHub](https://github.com/bitcoin-core/secp256k1)
* [Mastering Bitcoin](https://github.com/bitcoinbook/bitcoinbook)

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
It incorporates code from [libsecp256k1](https://github.com/bitcoin-core/secp256k1), also MIT licensed.

## Acknowledgments

* **Bitcoin Core Developers** — For libsecp256k1
* **ThrowTheSwitch** — For the Unity testing framework
