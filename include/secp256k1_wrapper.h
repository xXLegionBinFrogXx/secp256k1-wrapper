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

#ifndef SECP256K1_WRAPPER_H
#define SECP256K1_WRAPPER_H

#include <stddef.h>

#define SECP256K1_WRAPPER_VERSION_MAJOR 2
#define SECP256K1_WRAPPER_VERSION_MINOR 1
#define SECP256K1_WRAPPER_VERSION_PATCH 0

#define SECP256K1_WRAPPER_STR_HELPER(x) #x
#define SECP256K1_WRAPPER_STR(x) SECP256K1_WRAPPER_STR_HELPER(x)

// Define constants
#define SECP256K1_WRAPPER_PRIVKEY_SIZE 32
#define SECP256K1_WRAPPER_PUBKEY_COMPRESSION_SIZE 33
#define SECP256K1_WRAPPER_PUBKEY_UNCOMPRESSION_SIZE 65

/* ---- Compile-time size sanity checks ---- */
#if defined(__cplusplus) && (__cplusplus >= 201103L)
  static_assert(SECP256K1_WRAPPER_PRIVKEY_SIZE == 32, "SECP256K1_WRAPPER_PRIVKEY_SIZE must be 32");
  static_assert(SECP256K1_WRAPPER_PUBKEY_COMPRESSION_SIZE == 33, "SECP256K1_WRAPPER_PUBKEY_COMPRESSION_SIZE must be 33");
  static_assert(SECP256K1_WRAPPER_PUBKEY_UNCOMPRESSION_SIZE == 65, "SECP256K1_WRAPPER_PUBKEY_UNCOMPRESSION_SIZE must be 65");
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  _Static_assert(SECP256K1_WRAPPER_PRIVKEY_SIZE == 32, "SECP256K1_WRAPPER_PRIVKEY_SIZE must be 32");
  _Static_assert(SECP256K1_WRAPPER_PUBKEY_COMPRESSION_SIZE == 33, "SECP256K1_WRAPPER_PUBKEY_COMPRESSION_SIZE must be 33");
  _Static_assert(SECP256K1_WRAPPER_PUBKEY_UNCOMPRESSION_SIZE == 65, "SECP256K1_WRAPPER_PUBKEY_UNCOMPRESSION_SIZE must be 65");
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the version string of the library.
 * 
 * This function returns a pointer to a constant character string that represents
 * the version of the library. The version string is usually in the format "major.minor.patch",
 * e.g., "1.2.3".
 * 
 * @return A pointer to a null-terminated constant character string containing
 *         the version of the library.
 * 
 * @note The returned string is static and should not be modified. It is guaranteed
 *       to be valid for the duration of the program.
 */
const char* secp256k1_wrapper_get_version(void); 

/**
 * @brief Generates a secp256k1 private and public key pair.
 *
 * This function generates a random private key and its corresponding 
 * public key using the secp256k1 elliptic curve. The public key can be
 * either compressed (33 bytes) or uncompressed (65 bytes), depending on
 * the compressed parameter. It fills the provided output buffers with the
 * generated keys. It takes care of context creation and destruction.
 *
 * @param[out] privkey_out Pointer to a 32-byte array where the generated 
 *                         private key will be stored. This buffer must 
 *                         be pre-allocated by the caller. Contents are
 *                         undefined on failure.
 * @param[out] pubkey_out  Pointer to an array where the public key will be stored.
 *                         Must be at least pubkey_out_len bytes, and
 *                         pubkey_out_len must be at least 33 bytes for
 *                         compressed or 65 bytes for uncompressed. This
 *                         buffer must be pre-allocated. Contents are
 *                         undefined on failure.
 * @param[in] pubkey_out_len Size in bytes of the pubkey_out buffer.
 * @param[in] compressed   1 for compressed public key (33 bytes), 0 for
 *                         uncompressed public key (65 bytes). Any other
 *                         value returns -1.
 *
 * @return int Returns 0 on success, or a negative value on error:
 *             - -1: Invalid input (null buffers or invalid compressed value).
 *             - -2: Context creation failed.
 *             - -3: Random number generation failed.
 *             - -4: Context randomization failed.
 *             - -5: Public key creation failed.
 *             - -6: Public key serialization failed.
 *             - -8: pubkey_out_len is smaller than the size required by compressed.
 *
 * @note Callers that only check `result != 0` (or `result < 0`) for failure
 *       are unaffected by the specific code returned.
 *
 * @note pubkey_out_len is validated before any key material is generated.
 *       The private key buffer must still be sized correctly by the caller
 *       (32 bytes); this is not separately checked. On failure, the contents
 *       of output buffers are undefined and should not be used.
 *
 * @warning It is the caller's responsibility to ensure the output buffers
 *          are valid and to manage the keys securely after generation to
 *          prevent unintended exposure.
 */
int secp256k1_wrapper_generate_keys(unsigned char* privkey_out, unsigned char* pubkey_out, size_t pubkey_out_len, int compressed);


/**
 * @brief Derives a public key from a given private key.
 * 
 * This function derives a public key (compressed or uncompressed) from a given
 * 32-byte private key using the secp256k1 elliptic curve.
 * 
 * @param[in] privkey      A 32-byte private key.
 * @param[out] pubkey_out  A buffer to store the public key. Must be at least
 *                         pubkey_out_len bytes, and pubkey_out_len must be at
 *                         least 33 bytes for compressed or 65 bytes for
 *                         uncompressed. Must be pre-allocated. Contents are
 *                         undefined on failure.
 * @param[in] pubkey_out_len Size in bytes of the pubkey_out buffer.
 * @param[in] compressed   1 for compressed public key (33 bytes), 0 for
 *                         uncompressed public key (65 bytes). Any other
 *                         value returns -1.
 *
 * @return int Returns 0 on success, or a negative value on error:
 *             - -1: Invalid input (null buffers or invalid compressed value).
 *             - -2: Context creation failed.
 *             - -3: Random number generation failed.
 *             - -4: Context randomization failed.
 *             - -5: Public key creation failed.
 *             - -6: Public key serialization failed.
 *             - -7: Private key verification failed.
 *             - -8: pubkey_out_len is smaller than the size required by compressed.
 *
 * @note Callers that only check `result != 0` (or `result < 0`) for failure
 *       are unaffected by the specific code returned.
 *
 * @note On failure, the contents of pubkey_out are undefined and should not be used.
 */
int secp256k1_wrapper_derive_pubkey(const unsigned char* privkey, unsigned char* pubkey_out, size_t pubkey_out_len, int compressed);


/**
 * @brief Verifies that a 32-byte buffer holds a valid secp256k1 private key.
 *
 * A private key is valid if it is non-zero and strictly less than the curve
 * order n. This is a pure computation on public constants: it creates no
 * context, allocates no memory, and is safe to call concurrently from
 * multiple threads.
 *
 * @param[in] privkey A 32-byte private key to check.
 *
 * @return int Returns 0 if the key is valid, or a negative value on error:
 *             - -1: privkey is NULL.
 *             - -7: Private key verification failed (zero, or >= curve order n).
 *
 * @note Callers that only check `result != 0` (or `result < 0`) for failure
 *       are unaffected by the specific code returned.
 *
 * @warning This function runs libsecp256k1's self-test on every call as a
 *          build-sanity canary (e.g. to catch a library built for the wrong
 *          endianness). If the self-test fails, libsecp256k1's default error
 *          callback is invoked, which aborts the process. This does not
 *          happen on a correctly built library; it exists to fail fast on a
 *          broken one rather than silently returning invalid results.
 */
int secp256k1_wrapper_verify_privkey(const unsigned char* privkey);


/**
 * @brief Fills a buffer with random bytes.
 *
 * This function generates random bytes and fills the provided buffer with them.
 * The method of generating random bytes depends on the operating system:
 * - On Windows, it uses the BCryptGenRandom function.
 * - On Linux and FreeBSD, it uses the getrandom system call (with /dev/urandom fallback).
 * - On macOS, it uses CCRandomGenerateBytes (with getentropy fallback for ≤256 bytes).
 * - On OpenBSD, it uses the getentropy function (limited to 256 bytes).
 * 
 * @param[out] data A pointer to the buffer where the random bytes will be stored.
 * @param[in] size The number of random bytes to generate.
 * 
 * @return 1 if the buffer was successfully filled with random bytes, 0 otherwise.
 * 
 * @note 
 * - On Linux and FreeBSD, if `getrandom` fails with ENOSYS, falls back to /dev/urandom.
 * - On macOS, tries CCRandomGenerateBytes first, then getentropy for sizes ≤256 bytes.
 * - On OpenBSD, getentropy is limited to 256 bytes per call.
 * - On Windows, fails if the requested size is greater than `ULONG_MAX`.
 * 
 */
int secp256k1_wrapper_fill_random(unsigned char* data, size_t size);


/**
 * @brief Securely zeroes a buffer, guaranteed not to be optimized away.
 *
 * Uses the best available platform mechanism to ensure the write is not
 * elided by the compiler as a dead store:
 * - On Windows, SecureZeroMemory.
 * - Where available, the libc explicit_bzero function.
 * - Where C11 Annex K is implemented, memset_s.
 * - Otherwise, memset followed by a compiler barrier (GCC/Clang), or a
 *   volatile function pointer indirection as a last resort.
 *
 * @param[out] p   Pointer to the buffer to zero. If NULL, this is a no-op.
 * @param[in]  len Number of bytes to zero. If 0, this is a no-op.
 */
void secp256k1_wrapper_memzero(void* p, size_t len);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif // SECP256K1_WRAPPER_H
