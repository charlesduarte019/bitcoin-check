#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <secp256k1.h>
#include <openssl/sha.h>
#include <openssl/ripemd.h>

static void print_hex(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }

    printf("\n");
}

static int hex_to_32bytes(const char *hex, uint8_t out[32])
{
    size_t len = strlen(hex);

    if (len == 0 || len > 64)
        return 0;

    memset(out, 0, 32);

    size_t byte_len = (len + 1) / 2;
    size_t offset = 32 - byte_len;
    size_t src = 0;

    if (len & 1) {
        char tmp[2] = { hex[0], '\0' };
        char *end = NULL;

        unsigned long v = strtoul(tmp, &end, 16);

        if (*end != '\0')
            return 0;

        out[offset++] = (uint8_t)v;
        src = 1;
    }

    while (src < len) {
        char tmp[3] = {
            hex[src],
            hex[src + 1],
            '\0'
        };

        char *end = NULL;

        unsigned long v = strtoul(tmp, &end, 16);

        if (*end != '\0')
            return 0;

        out[offset++] = (uint8_t)v;

        src += 2;
    }

    return 1;
}

static void hash160(
    const uint8_t *data,
    size_t len,
    uint8_t output[20]
)
{
    uint8_t sha[32];

    SHA256(data, len, sha);
    RIPEMD160(sha, sizeof(sha), output);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(
            stderr,
            "Uso:\n"
            "  %s PRIVATE_KEY_HEX\n",
            argv[0]
        );

        return 1;
    }

    uint8_t private_key[32];

    if (!hex_to_32bytes(argv[1], private_key)) {
        fprintf(stderr, "Private key HEX invalida\n");
        return 1;
    }

    secp256k1_context *ctx =
        secp256k1_context_create(SECP256K1_CONTEXT_NONE);

    if (!ctx) {
        fprintf(stderr, "Falha ao criar contexto secp256k1\n");
        return 1;
    }

    if (!secp256k1_ec_seckey_verify(
        ctx,
        private_key
    )) {
        fprintf(stderr, "Private key invalida\n");

        secp256k1_context_destroy(ctx);
        return 1;
    }

    secp256k1_pubkey pubkey;

    if (!secp256k1_ec_pubkey_create(
        ctx,
        &pubkey,
        private_key
    )) {
        fprintf(stderr, "Falha ao gerar public key\n");

        secp256k1_context_destroy(ctx);
        return 1;
    }

    uint8_t compressed_pubkey[33];

    size_t compressed_len =
        sizeof(compressed_pubkey);

    secp256k1_ec_pubkey_serialize(
        ctx,
        compressed_pubkey,
        &compressed_len,
        &pubkey,
        SECP256K1_EC_COMPRESSED
    );

    uint8_t h160[20];

    hash160(
        compressed_pubkey,
        compressed_len,
        h160
    );

    printf("private_key=");
    print_hex(private_key, 32);

    printf("public_key=");
    print_hex(compressed_pubkey, compressed_len);

    printf("hash160=");
    print_hex(h160, 20);

    secp256k1_context_destroy(ctx);

    return 0;
}
