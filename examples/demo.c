#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <secp256k1_wrapper.h>  // provides PRIVKEY_SIZE, PUBKEY_*_SIZE and function prototypes

/* Simple hex printer */
static void print_hex(const unsigned char *data, size_t len) {
    for (size_t i = 0; i < len; i++) printf("%02X", data[i]);
    printf("\n");
}

/* Portable secure wipe */
static void secure_memzero(void *p, size_t n) {
    volatile unsigned char *vp = (volatile unsigned char *)p;
    while (n--) *vp++ = 0;
}

static int parse_compressed_arg(const char *s, int default_val) {
    if (!s) return default_val;
    if (strlen(s) == 1) {
        char c = (char)tolower((unsigned char)s[0]);
        if (c == 'c') return 1;  // compressed
        if (c == 'u') return 0;  // uncompressed
        if (c == '0') return 0;
        if (c == '1') return 1;
    }
    // Accept words too
    if (strcasecmp(s, "compressed") == 0) return 1;
    if (strcasecmp(s, "uncompressed") == 0) return 0;
    return default_val;
}

int main(int argc, char *argv[]) {
    int pairs_qty = 2;
    int compressed = 1; // default: compressed 33-byte pubkeys

    if (argc >= 2) {
        int n = atoi(argv[1]);
        if (n > 0) pairs_qty = n;
    }
    if (argc >= 3) {
        compressed = parse_compressed_arg(argv[2], 1);
    }

    size_t pubkey_len = compressed ? PUBKEY_COMPRESSION_SIZE : PUBKEY_UNCOMPRESSION_SIZE;

    printf("secp256k1_wrapper v%s\n", secp256k1_wrapper_get_version());
    printf("Generating %d key pair%s (%s pubkeys: %zu bytes)\n",
           pairs_qty, pairs_qty == 1 ? "" : "s",
           compressed ? "compressed" : "uncompressed",
           pubkey_len);

    for (int i = 0; i < pairs_qty; i++) {
        unsigned char privkey[PRIVKEY_SIZE];
        unsigned char pubkey[PUBKEY_UNCOMPRESSION_SIZE];         // max size (65)
        unsigned char derived_pubkey[PUBKEY_UNCOMPRESSION_SIZE]; // max size (65)

        memset(pubkey, 0, sizeof(pubkey));
        memset(derived_pubkey, 0, sizeof(derived_pubkey));

        int gres = secp256k1_wrapper_generate_keys(privkey, pubkey, compressed);
        if (gres != 0) {
            fprintf(stderr, "secp256k1_wrapper_generate_keys failed (%d) for pair %d\n", gres, i);
            continue;
        }

        int dres = secp256k1_wrapper_derive_pubkey(privkey, derived_pubkey, compressed);
        if (dres != 0) {
            fprintf(stderr, "secp256k1_wrapper_derive_pubkey failed (%d) for pair %d\n", dres, i);
            secure_memzero(privkey, sizeof(privkey));
            continue;
        }

        printf("Private Key [%d]:\n", i);
        print_hex(privkey, sizeof(privkey));

        printf("Public Key [%d]:\n", i);
        print_hex(pubkey, pubkey_len);

        printf("Derived Public Key [%d]:\n", i);
        print_hex(derived_pubkey, pubkey_len);

        if (memcmp(pubkey, derived_pubkey, pubkey_len) == 0) {
            printf("✅ Match confirmed for key %d\n\n", i);
        } else {
            printf("❌ Mismatch! Key %d\n\n", i);
        }

        /* Clean up sensitive material */
        secure_memzero(privkey, sizeof(privkey));
        secure_memzero(derived_pubkey, pubkey_len);
        secure_memzero(pubkey, pubkey_len);
    }

    return 0;
}
