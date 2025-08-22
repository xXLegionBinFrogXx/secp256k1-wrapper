/*
 * secp256k1_wrapper - convenience wrapper around libsecp256k1
 *
 * Copyright (c) 2025 xXLegionBinFrogXx
 *
 * This file is licensed under the MIT License.
 * See the LICENSE file in the project root for details.
 *
 * This project incorporates code from libsecp256k1,
 * copyright (c) 2013 Bitcoin Core Developers, MIT License.
 */

#define __STDC_WANT_LIB_EXT1__ 1

#include "secp256k1_wrapper.h"
#include "secp256k1.h"
#include <string.h>

#if defined(__cplusplus)
#error Trying to compile a C project with a C++ compiler.
#endif

/* ---------- Platform headers ---------- */

#if defined(_WIN32)
  /*
   * The defined WIN32_NO_STATUS macro disables return code definitions in
   * windows.h, which avoids "macro redefinition" MSVC warnings in ntstatus.h.
   */
  #define WIN32_NO_STATUS
  #include <windows.h>
  #undef WIN32_NO_STATUS
  #include <ntstatus.h>
  #include <bcrypt.h>
  #include <limits.h>
  #define HAVE_WIN_SECURE_ZERO 1
#elif defined(__linux__) || defined(__FreeBSD__)
  #include <sys/random.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <unistd.h>
#elif defined(__APPLE__)
  // CCRandomGenerateBytes (in libSystem)
  #include <CommonCrypto/CommonRandom.h>   
  #if defined(USE_SECURITY_RNG)
    #include <Security/SecRandom.h>        
  #endif
  #include <sys/random.h>               
  #include <unistd.h>
#elif defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__)
  #include <sys/random.h>  
  #include <unistd.h>
  #include <stdlib.h>                     
#else
  #error "Couldn't identify the OS"
#endif


/* Secure memory zeroing that won't be optimized away */
static void secure_memzero(void *p, size_t n) {
#if defined(_WIN32)
    SecureZeroMemory(p, n);
#elif defined(__STDC_LIB_EXT1__)
    (void)memset_s(p, n, 0, n);   // In case someone actually implemented section K
#elif defined(__GNUC__) || defined(__clang__)

    /* Let the compiler emit an efficient memset, then block DSE with a
       compiler barrier that "uses" ptr and clobbers memory. 
       This method is used in Linux kernel : `memzero_explicit()` see the `barrier_data`  
       ```
       static inline void memzero_explicit(void *s, size_t count){
	        memset(s, 0, count);
	        barrier_data(s);
        }
        ```*/

    memset(p, 0, n);
    __asm__ __volatile__ ("" : : "r"(p) : "memory");
#else
    /* Fallback. cannot be optimized away, I hope */
    typedef void *(*memset_fn)(void *, int, size_t);
    volatile memset_fn vmemset = memset;
    vmemset(p, 0, n);

#endif
}

const char* secp256k1_wrapper_get_version(void) {
    return STR(SECP256K1_WRAPPER_VERSION_MAJOR) "." STR(SECP256K1_WRAPPER_VERSION_MINOR) "." STR(SECP256K1_WRAPPER_VERSION_PATCH);
}

// Function that generates and returns both private and public keys
int secp256k1_wrapper_generate_keys(unsigned char* privkey_out, unsigned char* pubkey_out, int compressed) {

    if (privkey_out == NULL || pubkey_out == NULL || (compressed != 0 && compressed != 1)) {
        return -1; // Invalid input
    }

    size_t pubkey_len = compressed ? PUBKEY_COMPRESSION_SIZE : PUBKEY_UNCOMPRESSION_SIZE;
    int flags = compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED;


    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
    if (!ctx) {
        return -2; // Context creation failed
    }

    unsigned char randomize[PRIVKEY_SIZE];
    if (!secp256k1_wrapper_fill_random(randomize, sizeof(randomize))) {
        secp256k1_context_destroy(ctx);  // Clean up ctx before returning
        return -3; // Random number generation failed
    }

    /* Randomizing the context is recommended to protect against side-channel
     * leakage See `secp256k1_context_randomize` in secp256k1.h for more
     * information about it. This should never fail. */
    if (secp256k1_context_randomize(ctx, randomize) == 0){
        secure_memzero(randomize, sizeof(randomize)); 
        secp256k1_context_destroy(ctx); 
        return -2;// Context randomization failed
    }
    
    // Clear randomization data after use 
    secure_memzero(randomize, sizeof(randomize)); 

    // Generate private key
    unsigned char privkey[PRIVKEY_SIZE];
    do {
        if (!secp256k1_wrapper_fill_random(privkey, sizeof(privkey))) {
            secp256k1_context_destroy(ctx);
            return -3;  // Random number generation failed
        }
    } while (!secp256k1_ec_seckey_verify(ctx, privkey));

    // Create public key
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(ctx, &pubkey, privkey)) {
        secure_memzero(privkey, sizeof(privkey));
        secp256k1_context_destroy(ctx);
        return -5; // Public key creation failed
    }

    // Serialize public key in compressed or uncompresed format
    // on success write to the pubkey_out
    if (!secp256k1_ec_pubkey_serialize(ctx, pubkey_out, &pubkey_len, &pubkey, flags)) {
        secure_memzero(privkey, sizeof(privkey)); 
        secp256k1_context_destroy(ctx);
        return -5; // Public key serialization failed
    }
    
    // Safe since both are 32 bytes
    memcpy(privkey_out, privkey, sizeof(privkey)); 
    
    // Clean up on a way out
    secure_memzero(privkey, sizeof(privkey)); 
    secp256k1_context_destroy(ctx);
    return 0;
}


int secp256k1_wrapper_derive_pubkey(const unsigned char* privkey, unsigned char* pubkey_out, int compressed) {

    if (privkey == NULL || pubkey_out == NULL || (compressed != 0 && compressed != 1)) {
        return -1; // Invalid input
    }

    // Set public key length and serialization flag
    size_t pubkey_len = compressed ? PUBKEY_COMPRESSION_SIZE : PUBKEY_UNCOMPRESSION_SIZE;
    int flags = compressed ? SECP256K1_EC_COMPRESSED : SECP256K1_EC_UNCOMPRESSED;

    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
     if (!ctx) {
        return -2; // Context creation failed
    }

    if (!secp256k1_ec_seckey_verify(ctx, privkey)) {
        secp256k1_context_destroy(ctx);
        return -5; // Private key verification failed
    }

    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(ctx, &pubkey, privkey)) {
        secp256k1_context_destroy(ctx);
        return -5; // Public key creation failed
    }

    if (!secp256k1_ec_pubkey_serialize(ctx, pubkey_out, &pubkey_len, &pubkey, flags)) {
        secp256k1_context_destroy(ctx);
        return -5; // Public key serialization failed
    }

    secp256k1_context_destroy(ctx);
    return 0;
}

/* Returns 1 on success, and 0 on failure. */
int secp256k1_wrapper_fill_random(unsigned char* data, size_t size) {
#if defined(_WIN32)

    if (size > ULONG_MAX) return 0;
    NTSTATUS res = BCryptGenRandom(NULL, data, (ULONG)size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return res == STATUS_SUCCESS;

#elif defined(__linux__) || defined(__FreeBSD__)

    ssize_t res = getrandom(data, size, 0);
    if (res == -1 && errno == ENOSYS) {
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) {
            return 0;
        }
        size_t off = 0;
        while (off < size) {
            ssize_t r = read(fd, data + off, size - off);
            if (r <= 0) {
                close(fd);
                return 0;
            }
            off += (size_t)r;
        }
        close(fd);
        return 1;
    }
    return (res >= 0 && (size_t)res == size) ? 1 : 0;

#elif defined(__APPLE__)

    #if defined(USE_SECURITY_RNG)
        if (SecRandomCopyBytes(kSecRandomDefault, size, data) == errSecSuccess) return 1;
    #endif

    // Try CCRandomGenerateBytes (macOS 10.7+)
    if (CCRandomGenerateBytes(data, size) == kCCSuccess) {
        return 1;
    }
    // Try getentropy (macOS 10.12+), limited to 256 bytes
    #if defined(__MAC_10_12)
    if (size <= 256 && getentropy(data, size) == 0) {
        return 1;
    }
    #endif
   return 0;

#elif defined(__OpenBSD__)
    // getentropy() on OpenBSD is limited to 256 bytes per call
    if (size <= 256 && getentropy(data, size) == 0) {
        return 1;
    }
    return 0;

#else
    #error "Couldn't identify the OS"
#endif
}

