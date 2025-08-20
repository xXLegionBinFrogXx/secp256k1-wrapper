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

#include <stdlib.h>

#define SECP256K1_WRAPPER_VERSION_MAJOR 1
#define SECP256K1_WRAPPER_VERSION_MINOR 3
#define SECP256K1_WRAPPER_VERSION_PATCH 0

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

// Define constants
#define PRIVKEY_SIZE 32
#define PUBKEY_COMPRESSION_SIZE 33
#define PUBKEY_UNCOMPRESSION_SIZE 65

/* ---- Compile-time size sanity checks ---- */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
  _Static_assert(PRIVKEY_SIZE == 32, "PRIVKEY_SIZE must be 32");
  _Static_assert(PUBKEY_COMPRESSION_SIZE == 33, "PUBKEY_COMPRESSION_SIZE must be 33");
  _Static_assert(PUBKEY_UNCOMPRESSION_SIZE == 65, "PUBKEY_UNCOMPRESSION_SIZE must be 65");
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
 *                         Must be at least 33 bytes for compressed or 65 bytes
 *                         for uncompressed. This buffer must be pre-allocated.
 *                         Contents are undefined on failure.
 * @param[in] compressed   Non-zero for compressed public key (33 bytes),
 *                         zero for uncompressed public key (65 bytes).
 * 
 * @return int Returns 0 on success, or a negative value on error:
 *             - -1: Invalid input (null buffers or invalid compressed value).
 *             - -2: Context creation or randomization failed.
 *             - -3: Random number generation failed.
 *             - -5: Public key creation or serialization failed.
 *
 * @note The output buffers must be correctly sized (32 bytes for the 
 *       private key, 33 or 65 bytes for the public key) to avoid memory 
 *       corruption. On failure, the contents of output buffers are undefined
 *       and should not be used.
 *
 * @warning It is the caller's responsibility to ensure the output buffers 
 *          are valid and to manage the keys securely after generation to 
 *          prevent unintended exposure.
 */
int secp256k1_wrapper_generate_keys(unsigned char* privkey_out, unsigned char* pubkey_out, int compressed);


/**
 * @brief Derives a public key from a given private key.
 * 
 * This function derives a public key (compressed or uncompressed) from a given
 * 32-byte private key using the secp256k1 elliptic curve.
 * 
 * @param[in] privkey      A 32-byte private key.
 * @param[out] pubkey_out  A buffer to store the public key (33 bytes for compressed,
 *                         65 bytes for uncompressed). Must be pre-allocated.
 *                         Contents are undefined on failure.
 * @param[in] compressed   Non-zero for compressed public key (33 bytes),
 *                         zero for uncompressed public key (65 bytes).
 * 
 * @return int Returns 0 on success, or a negative value on error:
 *             - -1: Invalid input (null buffers or invalid compressed value).
 *             - -2: Context creation or randomization failed.
 *             - -5: Public key creation or serialization failed.
 * 
 * @note On failure, the contents of pubkey_out are undefined and should not be used.
 */
int secp256k1_wrapper_derive_pubkey(const unsigned char* privkey, unsigned char* pubkey_out,int compressed);


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

#endif // SECP256K1_WRAPPER_H
