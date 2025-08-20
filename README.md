# secp256k1-wrapper

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![CMake](https://img.shields.io/badge/CMake-3.15+-blue.svg)](https://cmake.org/)
[![C Standard](https://img.shields.io/badge/C-C99-blue.svg)](https://en.wikipedia.org/wiki/C99)

A modern, lightweight C wrapper library for [libsecp256k1](https://github.com/bitcoin-core/secp256k1) that simplifies elliptic curve cryptography operations for secp256k1 keys.

## Features

| Feature | Description |
|---------|-------------|
| 🛠️ **Simple API** | Easy-to-use functions for key generation and derivation |
| 🌐 **Cross-Platform** | Supports Windows, macOS, Linux, and BSD |
| 🔒 **Secure** | Platform-specific secure random number generation |
| 🧵 **Thread-Safe** | No global state, context management per operation |
| 🔧 **Modern Build** | CMake-based build system using latest secp256k1 v0.7.0 features |
| ✅ **Well Tested** | Comprehensive test suite with Unity v2.6.1 framework |
| 📦 **No Manual Setup** | Dependencies auto-fetched via CMake, no git submodules or manual setup |

## For Cryptography Beginners

| Term | Description |
|------|-------------|
| **secp256k1** | An elliptic curve used in Bitcoin for secure key generation and digital signatures |
| **Private Key** | A 32-byte secret number used to sign transactions - keep this secure! |
| **Public Key** | Derived from the private key, either 33 bytes (compressed) or 65 bytes (uncompressed) |
| **Key Derivation** | Process of generating a public key from a private key |
| **Compressed vs Uncompressed** | Two formats for public keys - compressed saves space (33 vs 65 bytes) |

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

### C Usage

```c
#include <secp256k1_wrapper.h>
#include <stdio.h>

int main() {
    // Generate a new key pair
    unsigned char privkey[32];
    unsigned char pubkey[33];  // 33 bytes for compressed, 65 for uncompressed
    
    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 1); // 1 = compressed
    if (result != 0) {
        printf("Key generation failed: %d\n", result);
        return 1;
    }
    
    // Derive public key from existing private key
    unsigned char derived_pubkey[33];
    result = secp256k1_wrapper_derive_pubkey(privkey, derived_pubkey, 1);
    
    // Get library version
    const char* version = secp256k1_wrapper_get_version();
    printf("Using secp256k1-wrapper v%s\n", version);
    
    // Generate secure random bytes
    unsigned char random_data[32];
    int success = secp256k1_wrapper_fill_random(random_data, 32);
    
    return 0;
}
```

### Go (CGO) Usage

```go
package main

/*
#cgo CFLAGS: -I/usr/local/include
#cgo LDFLAGS: -L/usr/local/lib -lsecp256k1-wrapper
#include <secp256k1_wrapper.h>
#include <stdlib.h>
*/
import "C"
import (
    "fmt"
    "unsafe"
)

func main() {
    // Generate a new key pair
    var privkey [32]C.uchar
    var pubkey [33]C.uchar
    
    result := C.secp256k1_wrapper_generate_keys(&privkey[0], &pubkey[0], 1)
    if result != 0 {
        fmt.Printf("Key generation failed: %d\n", result)
        return
    }
    
    // Convert to Go slices for easier handling
    privkeySlice := C.GoBytes(unsafe.Pointer(&privkey[0]), 32)
    pubkeySlice := C.GoBytes(unsafe.Pointer(&pubkey[0]), 33)
    
    fmt.Printf("Private key: %x\n", privkeySlice)
    fmt.Printf("Public key: %x\n", pubkeySlice)
    
    // Get library version
    version := C.secp256k1_wrapper_get_version()
    fmt.Printf("Using secp256k1-wrapper v%s\n", C.GoString(version))
}
```

### C++ Usage

```cpp
#include <secp256k1_wrapper.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <array>

class Secp256k1Wrapper {
public:
    static std::string getVersion() {
        return std::string(secp256k1_wrapper_get_version());
    }
    
    static std::pair<std::array<uint8_t, 32>, std::vector<uint8_t>> 
    generateKeys(bool compressed = true) {
        std::array<uint8_t, 32> privkey;
        std::vector<uint8_t> pubkey(compressed ? 33 : 65);
        
        int result = secp256k1_wrapper_generate_keys(
            privkey.data(), 
            pubkey.data(), 
            compressed ? 1 : 0
        );
        
        if (result != 0) {
            throw std::runtime_error("Key generation failed: " + std::to_string(result));
        }
        
        return {privkey, pubkey};
    }
    
    static std::vector<uint8_t> derivePublicKey(
        const std::array<uint8_t, 32>& privkey, 
        bool compressed = true
    ) {
        std::vector<uint8_t> pubkey(compressed ? 33 : 65);
        
        int result = secp256k1_wrapper_derive_pubkey(
            privkey.data(), 
            pubkey.data(), 
            compressed ? 1 : 0
        );
        
        if (result != 0) {
            throw std::runtime_error("Public key derivation failed: " + std::to_string(result));
        }
        
        return pubkey;
    }
};

int main() {
    try {
        std::cout << "Using " << Secp256k1Wrapper::getVersion() << std::endl;
        
        // Generate key pair
        auto [privkey, pubkey] = Secp256k1Wrapper::generateKeys(true);
        
        // Print keys in hex
        std::cout << "Private key: ";
        for (const auto& byte : privkey) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                     << static_cast<int>(byte);
        }
        std::cout << std::endl;
        
        std::cout << "Public key: ";
        for (const auto& byte : pubkey) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                     << static_cast<int>(byte);
        }
        std::cout << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
```

### Python (ctypes) Usage

```python
import ctypes
import os
from typing import Tuple, List

class Secp256k1Wrapper:
    def __init__(self, lib_path: str = "libsecp256k1-wrapper.so"):
        """Load the secp256k1-wrapper library"""
        self.lib = ctypes.CDLL(lib_path)
        
        # Define function signatures
        self.lib.secp256k1_wrapper_get_version.restype = ctypes.c_char_p
        
        self.lib.secp256k1_wrapper_generate_keys.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte), 
            ctypes.POINTER(ctypes.c_ubyte), 
            ctypes.c_int
        ]
        self.lib.secp256k1_wrapper_generate_keys.restype = ctypes.c_int
        
        self.lib.secp256k1_wrapper_derive_pubkey.argtypes = [
            ctypes.POINTER(ctypes.c_ubyte), 
            ctypes.POINTER(ctypes.c_ubyte), 
            ctypes.c_int
        ]
        self.lib.secp256k1_wrapper_derive_pubkey.restype = ctypes.c_int
    
    def get_version(self) -> str:
        """Get library version string"""
        return self.lib.secp256k1_wrapper_get_version().decode('utf-8')
    
    def generate_keys(self, compressed: bool = True) -> Tuple[bytes, bytes]:
        """Generate a new key pair"""
        privkey = (ctypes.c_ubyte * 32)()
        pubkey_size = 33 if compressed else 65
        pubkey = (ctypes.c_ubyte * pubkey_size)()
        
        result = self.lib.secp256k1_wrapper_generate_keys(
            privkey, pubkey, 1 if compressed else 0
        )
        
        if result != 0:
            raise RuntimeError(f"Key generation failed: {result}")
        
        return (bytes(privkey), bytes(pubkey))
    
    def derive_public_key(self, privkey: bytes, compressed: bool = True) -> bytes:
        """Derive public key from private key"""
        if len(privkey) != 32:
            raise ValueError("Private key must be 32 bytes")
        
        privkey_array = (ctypes.c_ubyte * 32)(*privkey)
        pubkey_size = 33 if compressed else 65
        pubkey = (ctypes.c_ubyte * pubkey_size)()
        
        result = self.lib.secp256k1_wrapper_derive_pubkey(
            privkey_array, pubkey, 1 if compressed else 0
        )
        
        if result != 0:
            raise RuntimeError(f"Public key derivation failed: {result}")
        
        return bytes(pubkey)

# Example usage
if __name__ == "__main__":
    wrapper = Secp256k1Wrapper()
    
    print(f"Using {wrapper.get_version()}")
    
    # Generate key pair
    privkey, pubkey = wrapper.generate_keys(compressed=True)
    
    print(f"Private key: {privkey.hex()}")
    print(f"Public key: {pubkey.hex()}")
    
    # Derive public key from private key
    derived_pubkey = wrapper.derive_public_key(privkey, compressed=True)
    print(f"Derived public key: {derived_pubkey.hex()}")
    
    # Verify they match
    assert pubkey == derived_pubkey, "Keys should match!"
    print("✅ Key derivation verified!")
```

### Rust (FFI) Usage

```rust
use std::ffi::CStr;
use std::os::raw::{c_char, c_int, c_uchar};

#[link(name = "secp256k1-wrapper")]
extern "C" {
    fn secp256k1_wrapper_get_version() -> *const c_char;
    fn secp256k1_wrapper_generate_keys(
        privkey: *mut c_uchar,
        pubkey: *mut c_uchar,
        compressed: c_int,
    ) -> c_int;
    fn secp256k1_wrapper_derive_pubkey(
        privkey: *const c_uchar,
        pubkey: *mut c_uchar,
        compressed: c_int,
    ) -> c_int;
}

pub struct Secp256k1Wrapper;

impl Secp256k1Wrapper {
    pub fn version() -> String {
        unsafe {
            let version_ptr = secp256k1_wrapper_get_version();
            let version_cstr = CStr::from_ptr(version_ptr);
            version_cstr.to_string_lossy().into_owned()
        }
    }
    
    pub fn generate_keys(compressed: bool) -> Result<([u8; 32], Vec<u8>), i32> {
        let mut privkey = [0u8; 32];
        let pubkey_size = if compressed { 33 } else { 65 };
        let mut pubkey = vec![0u8; pubkey_size];
        
        let result = unsafe {
            secp256k1_wrapper_generate_keys(
                privkey.as_mut_ptr(),
                pubkey.as_mut_ptr(),
                if compressed { 1 } else { 0 },
            )
        };
        
        if result == 0 {
            Ok((privkey, pubkey))
        } else {
            Err(result)
        }
    }
    
    pub fn derive_public_key(privkey: &[u8; 32], compressed: bool) -> Result<Vec<u8>, i32> {
        let pubkey_size = if compressed { 33 } else { 65 };
        let mut pubkey = vec![0u8; pubkey_size];
        
        let result = unsafe {
            secp256k1_wrapper_derive_pubkey(
                privkey.as_ptr(),
                pubkey.as_mut_ptr(),
                if compressed { 1 } else { 0 },
            )
        };
        
        if result == 0 {
            Ok(pubkey)
        } else {
            Err(result)
        }
    }
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    println!("Using {}", Secp256k1Wrapper::version());
    
    // Generate key pair
    let (privkey, pubkey) = Secp256k1Wrapper::generate_keys(true)
        .map_err(|e| format!("Key generation failed: {}", e))?;
    
    println!("Private key: {}", hex::encode(privkey));
    println!("Public key: {}", hex::encode(&pubkey));
    
    // Derive public key from private key
    let derived_pubkey = Secp256k1Wrapper::derive_public_key(&privkey, true)
        .map_err(|e| format!("Public key derivation failed: {}", e))?;
    
    println!("Derived public key: {}", hex::encode(&derived_pubkey));
    
    // Verify they match
    assert_eq!(pubkey, derived_pubkey);
    println!("✅ Key derivation verified!");
    
    Ok(())
}
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
- **Unity v2.6.1** - Test framework (automatically fetched when BUILD_TESTS=ON)

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

## Error Handling Examples

### C Error Handling

```c
#include <secp256k1_wrapper.h>
#include <stdio.h>

const char* get_error_message(int error_code) {
    switch (error_code) {
        case 0: return "Success";
        case -1: return "Invalid input parameters";
        case -2: return "Context creation/randomization failed";
        case -3: return "Random number generation failed";
        case -5: return "Public key creation/serialization failed";
        default: return "Unknown error";
    }
}

int main() {
    unsigned char privkey[32];
    unsigned char pubkey[33];
    
    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 1);
    if (result != 0) {
        fprintf(stderr, "Error: %s (code: %d)\n", get_error_message(result), result);
        return 1;
    }
    
    printf("✅ Keys generated successfully\n");
    return 0;
}
```

### Go Error Handling

```go
package main

import (
    "errors"
    "fmt"
)

func getErrorMessage(code int) error {
    switch code {
    case 0:
        return nil
    case -1:
        return errors.New("invalid input parameters")
    case -2:
        return errors.New("context creation/randomization failed")
    case -3:
        return errors.New("random number generation failed")
    case -5:
        return errors.New("public key creation/serialization failed")
    default:
        return fmt.Errorf("unknown error (code: %d)", code)
    }
}

func generateKeys() error {
    var privkey [32]C.uchar
    var pubkey [33]C.uchar
    
    result := int(C.secp256k1_wrapper_generate_keys(&privkey[0], &pubkey[0], 1))
    if err := getErrorMessage(result); err != nil {
        return fmt.Errorf("key generation failed: %w", err)
    }
    
    fmt.Println("✅ Keys generated successfully")
    return nil
}
```

### C++ Error Handling

```cpp
#include <secp256k1_wrapper.h>
#include <stdexcept>
#include <string>

class Secp256k1Error : public std::runtime_error {
public:
    explicit Secp256k1Error(int code) 
        : std::runtime_error(getErrorMessage(code)), error_code_(code) {}
    
    int code() const noexcept { return error_code_; }
    
private:
    int error_code_;
    
    static std::string getErrorMessage(int code) {
        switch (code) {
            case -1: return "Invalid input parameters";
            case -2: return "Context creation/randomization failed";
            case -3: return "Random number generation failed";
            case -5: return "Public key creation/serialization failed";
            default: return "Unknown error (code: " + std::to_string(code) + ")";
        }
    }
};

std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 33>> 
generateKeysWithErrorHandling() {
    std::array<uint8_t, 32> privkey;
    std::array<uint8_t, 33> pubkey;
    
    int result = secp256k1_wrapper_generate_keys(privkey.data(), pubkey.data(), 1);
    if (result != 0) {
        throw Secp256k1Error(result);
    }
    
    return {privkey, pubkey};
}

int main() {
    try {
        auto [privkey, pubkey] = generateKeysWithErrorHandling();
        std::cout << "✅ Keys generated successfully" << std::endl;
    } catch (const Secp256k1Error& e) {
        std::cerr << "Error: " << e.what() << " (code: " << e.code() << ")" << std::endl;
        return 1;
    }
    return 0;
}
```

### Python Error Handling

```python
class Secp256k1Error(Exception):
    """Custom exception for secp256k1-wrapper errors"""
    
    ERROR_MESSAGES = {
        -1: "Invalid input parameters",
        -2: "Context creation/randomization failed", 
        -3: "Random number generation failed",
        -5: "Public key creation/serialization failed"
    }
    
    def __init__(self, code: int):
        self.code = code
        message = self.ERROR_MESSAGES.get(code, f"Unknown error (code: {code})")
        super().__init__(message)

class SafeSecp256k1Wrapper(Secp256k1Wrapper):
    """Enhanced wrapper with better error handling"""
    
    def generate_keys(self, compressed: bool = True) -> Tuple[bytes, bytes]:
        """Generate keys with detailed error handling"""
        privkey = (ctypes.c_ubyte * 32)()
        pubkey_size = 33 if compressed else 65
        pubkey = (ctypes.c_ubyte * pubkey_size)()
        
        result = self.lib.secp256k1_wrapper_generate_keys(
            privkey, pubkey, 1 if compressed else 0
        )
        
        if result != 0:
            raise Secp256k1Error(result)
        
        return (bytes(privkey), bytes(pubkey))

# Usage with error handling
try:
    wrapper = SafeSecp256k1Wrapper()
    privkey, pubkey = wrapper.generate_keys(compressed=True)
    print("✅ Keys generated successfully")
    print(f"Private key: {privkey.hex()}")
    print(f"Public key: {pubkey.hex()}")
    
except Secp256k1Error as e:
    print(f"❌ Error: {e} (code: {e.code})")
except Exception as e:
    print(f"❌ Unexpected error: {e}")
```

### Rust Error Handling

```rust
use std::fmt;

#[derive(Debug)]
pub enum Secp256k1Error {
    InvalidParameters,
    ContextCreationFailed,
    RandomGenerationFailed,
    PublicKeyCreationFailed,
    UnknownError(i32),
}

impl fmt::Display for Secp256k1Error {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            Secp256k1Error::InvalidParameters => write!(f, "Invalid input parameters"),
            Secp256k1Error::ContextCreationFailed => write!(f, "Context creation/randomization failed"),
            Secp256k1Error::RandomGenerationFailed => write!(f, "Random number generation failed"),
            Secp256k1Error::PublicKeyCreationFailed => write!(f, "Public key creation/serialization failed"),
            Secp256k1Error::UnknownError(code) => write!(f, "Unknown error (code: {})", code),
        }
    }
}

impl From<i32> for Secp256k1Error {
    fn from(code: i32) -> Self {
        match code {
            -1 => Secp256k1Error::InvalidParameters,
            -2 => Secp256k1Error::ContextCreationFailed,
            -3 => Secp256k1Error::RandomGenerationFailed,
            -5 => Secp256k1Error::PublicKeyCreationFailed,
            _ => Secp256k1Error::UnknownError(code),
        }
    }
}

impl Secp256k1Wrapper {
    pub fn generate_keys_safe(compressed: bool) -> Result<([u8; 32], Vec<u8>), Secp256k1Error> {
        match Self::generate_keys(compressed) {
            Ok(keys) => Ok(keys),
            Err(code) => Err(Secp256k1Error::from(code)),
        }
    }
}

fn main() {
    match Secp256k1Wrapper::generate_keys_safe(true) {
        Ok((privkey, pubkey)) => {
            println!("✅ Keys generated successfully");
            println!("Private key: {}", hex::encode(privkey));
            println!("Public key: {}", hex::encode(&pubkey));
        }
        Err(e) => {
            eprintln!("❌ Error: {}", e);
            std::process::exit(1);
        }
    }
}
```

## Resources

### Learn More About Cryptography
- 📚 [Elliptic Curve Cryptography Guide](https://en.wikipedia.org/wiki/Elliptic_Curve_Digital_Signature_Algorithm) - Understanding the mathematics behind ECC
- 🔐 [Practical Cryptography for Developers](https://cryptobook.nakov.com/) - Comprehensive guide to modern cryptography
- 📖 [Introduction to secp256k1](https://en.bitcoin.it/wiki/Secp256k1) - Bitcoin Wiki explanation

### Library Documentation
- 🛠️ [libsecp256k1 GitHub](https://github.com/bitcoin-core/secp256k1) - Official Bitcoin secp256k1 library
- 📋 [secp256k1 API Reference](https://github.com/bitcoin-core/secp256k1/blob/master/include/secp256k1.h) - Complete function documentation
- ⚙️ [CMake FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) - Understanding dependency management

### Bitcoin Development
- 🪙 [Bitcoin Developer Guide](https://developer.bitcoin.org/) - Official Bitcoin development resources
- 💻 [Mastering Bitcoin (Book)](https://github.com/bitcoinbook/bitcoinbook) - Free online book about Bitcoin internals
- 🔧 [Bitcoin Core Developer Documentation](https://bitcoincore.org/en/doc/) - Technical documentation

### Testing & Development
- ✅ [Unity Testing Framework](https://github.com/ThrowTheSwitch/Unity) - C unit testing framework we use
- 🏗️ [Modern CMake Practices](https://cliutils.gitlab.io/modern-cmake/) - Best practices for CMake

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

This project incorporates code from [libsecp256k1](https://github.com/bitcoin-core/secp256k1), which is also MIT licensed.

## Acknowledgments

- **Bitcoin Core Developers** - For the excellent libsecp256k1 library
- **ThrowTheSwitch** - For the Unity testing framework
