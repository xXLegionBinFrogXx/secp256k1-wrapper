#include <unity.h>
#include <string.h>
#include <stdio.h>
#include "secp256k1_wrapper.h"  
#include <secp256k1.h>

/* Secure memory zeroing */
static void secure_memzero(void *p, size_t n) {
    volatile unsigned char *vp = (volatile unsigned char *)p;
    while (n--) *vp++ = 0;
}

/* Helper function to verify if key is proper private key */
static int is_valid_privkey(const unsigned char* privkey) {
    secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
    int result = secp256k1_ec_seckey_verify(ctx, privkey);
    secp256k1_context_destroy(ctx);
    return result;
}

/* Helper to check if buffer contains all zeros */
static int is_all_zeros(const unsigned char* buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (buf[i] != 0) return 0;
    }
    return 1;
}

void setUp(void) {
    // Setup code if needed
}

void tearDown(void) {
    // Cleanup code if needed
}

/* ========== Basic Functionality Tests ========== */

void test_generate_compressed_keys(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
    
    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 1);  // 1 = compressed
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, is_valid_privkey(privkey));
    TEST_ASSERT_EQUAL_INT(0, is_all_zeros(pubkey, PUBKEY_COMPRESSION_SIZE));
    
    secure_memzero(privkey, sizeof(privkey));
}

void test_generate_uncompressed_keys(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char pubkey[PUBKEY_UNCOMPRESSION_SIZE];
    
    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 0);  // 0 = uncompressed
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(1, is_valid_privkey(privkey));
    TEST_ASSERT_EQUAL_INT(0, is_all_zeros(pubkey, PUBKEY_UNCOMPRESSION_SIZE));
    
    // Verify uncompressed key starts with 0x04
    TEST_ASSERT_EQUAL_HEX8(0x04, pubkey[0]);
    
    secure_memzero(privkey, sizeof(privkey));
}

/* ========== derive_pubkey Tests ========== */

void test_derive_pubkey_compressed(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char generated_pubkey[PUBKEY_COMPRESSION_SIZE];
    unsigned char derived_pubkey[PUBKEY_COMPRESSION_SIZE];
    
    // Generate a key pair
    int gen_result = secp256k1_wrapper_generate_keys(privkey, generated_pubkey, 1);
    TEST_ASSERT_EQUAL_INT(0, gen_result);
    
    // Derive public key from the same private key
    int derive_result = secp256k1_wrapper_derive_pubkey(privkey, derived_pubkey, 1);
    TEST_ASSERT_EQUAL_INT(0, derive_result);
    
    // They should match exactly
    TEST_ASSERT_EQUAL_MEMORY(generated_pubkey, derived_pubkey, PUBKEY_COMPRESSION_SIZE);
    
    secure_memzero(privkey, sizeof(privkey));
}

void test_derive_pubkey_uncompressed(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char generated_pubkey[PUBKEY_UNCOMPRESSION_SIZE];
    unsigned char derived_pubkey[PUBKEY_UNCOMPRESSION_SIZE];
    
    // Generate a key pair
    int gen_result = secp256k1_wrapper_generate_keys(privkey, generated_pubkey, 0);
    TEST_ASSERT_EQUAL_INT(0, gen_result);
    
    // Derive public key from the same private key
    int derive_result = secp256k1_wrapper_derive_pubkey(privkey, derived_pubkey, 0);
    TEST_ASSERT_EQUAL_INT(0, derive_result);
    
    // They should match exactly
    TEST_ASSERT_EQUAL_MEMORY(generated_pubkey, derived_pubkey, PUBKEY_UNCOMPRESSION_SIZE);
    
    secure_memzero(privkey, sizeof(privkey));
}

void test_same_privkey_different_formats(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char compressed_pubkey[PUBKEY_COMPRESSION_SIZE];
    unsigned char uncompressed_pubkey[PUBKEY_UNCOMPRESSION_SIZE];
    unsigned char derived_compressed[PUBKEY_COMPRESSION_SIZE];
    unsigned char derived_uncompressed[PUBKEY_UNCOMPRESSION_SIZE];
    
    // Generate compressed keys
    int result = secp256k1_wrapper_generate_keys(privkey, compressed_pubkey, 1);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    // Derive both formats from the same private key
    result = secp256k1_wrapper_derive_pubkey(privkey, derived_compressed, 1);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    result = secp256k1_wrapper_derive_pubkey(privkey, derived_uncompressed, 0);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    // Compressed should match
    TEST_ASSERT_EQUAL_MEMORY(compressed_pubkey, derived_compressed, PUBKEY_COMPRESSION_SIZE);
    
    // Both should be valid and related (can't directly compare due to different formats)
    TEST_ASSERT_EQUAL_HEX8(0x04, derived_uncompressed[0]);  // Uncompressed prefix
    TEST_ASSERT_TRUE(compressed_pubkey[0] == 0x02 || compressed_pubkey[0] == 0x03);  // Compressed prefix
    
    secure_memzero(privkey, sizeof(privkey));
}

/* ========== Error Handling Tests ========== */

void test_generate_keys_null_privkey(void) {
    unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
    int result = secp256k1_wrapper_generate_keys(NULL, pubkey, 1);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_generate_keys_null_pubkey(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    int result = secp256k1_wrapper_generate_keys(privkey, NULL, 1);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_generate_keys_invalid_compressed(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
    
    // Test with invalid compressed values
    int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 2);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = secp256k1_wrapper_generate_keys(privkey, pubkey, -1);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = secp256k1_wrapper_generate_keys(privkey, pubkey, 255);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_derive_pubkey_null_privkey(void) {
    unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
    int result = secp256k1_wrapper_derive_pubkey(NULL, pubkey, 1);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_derive_pubkey_null_pubkey(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    memset(privkey, 0x42, sizeof(privkey));  // Some valid-looking data
    int result = secp256k1_wrapper_derive_pubkey(privkey, NULL, 1);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_derive_pubkey_invalid_compressed(void) {
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
    
    // Generate a valid private key first
    int gen_result = secp256k1_wrapper_generate_keys(privkey, pubkey, 1);
    TEST_ASSERT_EQUAL_INT(0, gen_result);
    
    // Test with invalid compressed values
    int result = secp256k1_wrapper_derive_pubkey(privkey, pubkey, 2);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    result = secp256k1_wrapper_derive_pubkey(privkey, pubkey, -1);
    TEST_ASSERT_EQUAL_INT(-1, result);
    
    secure_memzero(privkey, sizeof(privkey));
}

void test_derive_pubkey_invalid_privkey(void) {
    unsigned char invalid_privkey[PRIVKEY_SIZE];
    unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
    
    // All zeros is invalid
    memset(invalid_privkey, 0x00, sizeof(invalid_privkey));
    int result = secp256k1_wrapper_derive_pubkey(invalid_privkey, pubkey, 1);
    TEST_ASSERT_EQUAL_INT(-5, result);
    
    // All 0xFF is likely invalid (exceeds curve order)
    memset(invalid_privkey, 0xFF, sizeof(invalid_privkey));
    result = secp256k1_wrapper_derive_pubkey(invalid_privkey, pubkey, 1);
    TEST_ASSERT_EQUAL_INT(-5, result);
}

/* ========== Stress Tests ========== */

void test_stress_compressed_generation(void) {
    const int num_keys = 100;
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
    
    for (int i = 0; i < num_keys; i++) {
        memset(privkey, 0, sizeof(privkey));
        memset(pubkey, 0, sizeof(pubkey));
        
        int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 1);
        TEST_ASSERT_EQUAL_INT(0, result);
        TEST_ASSERT_EQUAL_INT(1, is_valid_privkey(privkey));
        TEST_ASSERT_EQUAL_INT(0, is_all_zeros(pubkey, PUBKEY_COMPRESSION_SIZE));
        
        secure_memzero(privkey, sizeof(privkey));
    }
}

void test_stress_uncompressed_generation(void) {
    const int num_keys = 100;
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char pubkey[PUBKEY_UNCOMPRESSION_SIZE];
    
    for (int i = 0; i < num_keys; i++) {
        memset(privkey, 0, sizeof(privkey));
        memset(pubkey, 0, sizeof(pubkey));
        
        int result = secp256k1_wrapper_generate_keys(privkey, pubkey, 0);
        TEST_ASSERT_EQUAL_INT(0, result);
        TEST_ASSERT_EQUAL_INT(1, is_valid_privkey(privkey));
        TEST_ASSERT_EQUAL_INT(0, is_all_zeros(pubkey, PUBKEY_UNCOMPRESSION_SIZE));
        TEST_ASSERT_EQUAL_HEX8(0x04, pubkey[0]);
        
        secure_memzero(privkey, sizeof(privkey));
    }
}

void test_key_uniqueness(void) {
    const int num_keys = 50;
    unsigned char keys[num_keys][PRIVKEY_SIZE];
    
    // Generate multiple keys
    for (int i = 0; i < num_keys; i++) {
        unsigned char pubkey[PUBKEY_COMPRESSION_SIZE];
        int result = secp256k1_wrapper_generate_keys(keys[i], pubkey, 1);
        TEST_ASSERT_EQUAL_INT(0, result);
    }
    
    // Check for uniqueness
    for (int i = 0; i < num_keys; i++) {
        for (int j = i + 1; j < num_keys; j++) {
            TEST_ASSERT_FALSE(memcmp(keys[i], keys[j], PRIVKEY_SIZE) == 0);
        }
    }
    
    // Clean up
    secure_memzero(keys, sizeof(keys));
}

/* ========== Known Test Vectors ========== */

void test_known_private_key(void) {
    // Test with a known private key (all 0x01 bytes, which is valid)
    unsigned char privkey[PRIVKEY_SIZE];
    unsigned char pubkey_compressed[PUBKEY_COMPRESSION_SIZE];
    unsigned char pubkey_uncompressed[PUBKEY_UNCOMPRESSION_SIZE];
    
    // Set private key to 0x0000...0001 (valid private key)
    memset(privkey, 0x00, sizeof(privkey));
    privkey[31] = 0x01;
    
    int result = secp256k1_wrapper_derive_pubkey(privkey, pubkey_compressed, 1);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    result = secp256k1_wrapper_derive_pubkey(privkey, pubkey_uncompressed, 0);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    // Known public key for private key = 1 (generator point)
    // Compressed: 0279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798
    unsigned char expected_compressed[33] = {
        0x02, 0x79, 0xBE, 0x66, 0x7E, 0xF9, 0xDC, 0xBB,
        0xAC, 0x55, 0xA0, 0x62, 0x95, 0xCE, 0x87, 0x0B,
        0x07, 0x02, 0x9B, 0xFC, 0xDB, 0x2D, 0xCE, 0x28,
        0xD9, 0x59, 0xF2, 0x81, 0x5B, 0x16, 0xF8, 0x17, 0x98
    };
    
    TEST_ASSERT_EQUAL_MEMORY(expected_compressed, pubkey_compressed, PUBKEY_COMPRESSION_SIZE);
    
    secure_memzero(privkey, sizeof(privkey));
}

/* ========== Version Test ========== */

void test_version_format(void) {
    const char* version = secp256k1_wrapper_get_version();
    TEST_ASSERT_NOT_NULL(version);
    
    // Check format: should be X.Y.Z
    int major, minor, patch;
    int parsed = sscanf(version, "%d.%d.%d", &major, &minor, &patch);
    TEST_ASSERT_EQUAL_INT(3, parsed);
    TEST_ASSERT_TRUE(major >= 0);
    TEST_ASSERT_TRUE(minor >= 0);
    TEST_ASSERT_TRUE(patch >= 0);
}

/* ========== Main Test Runner ========== */

int main(void) {
    UNITY_BEGIN();
    
    // Basic functionality
    RUN_TEST(test_generate_compressed_keys);
    RUN_TEST(test_generate_uncompressed_keys);
    
    // derive_pubkey tests
    RUN_TEST(test_derive_pubkey_compressed);
    RUN_TEST(test_derive_pubkey_uncompressed);
    RUN_TEST(test_same_privkey_different_formats);
    
    // Error handling
    RUN_TEST(test_generate_keys_null_privkey);
    RUN_TEST(test_generate_keys_null_pubkey);
    RUN_TEST(test_generate_keys_invalid_compressed);
    RUN_TEST(test_derive_pubkey_null_privkey);
    RUN_TEST(test_derive_pubkey_null_pubkey);
    RUN_TEST(test_derive_pubkey_invalid_compressed);
    RUN_TEST(test_derive_pubkey_invalid_privkey);
    
    // Stress tests
    RUN_TEST(test_stress_compressed_generation);
    RUN_TEST(test_stress_uncompressed_generation);
    RUN_TEST(test_key_uniqueness);
    
    // Known vectors
    RUN_TEST(test_known_private_key);
    
    // Version test
    RUN_TEST(test_version_format);
    
    return UNITY_END();
}
