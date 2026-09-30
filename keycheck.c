#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <secp256k1.h>

#include <openssl/sha.h>
#include <openssl/ripemd.h>

static const char *BASE58 =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

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

    if (len == 0 || len > 64) {
        return 0;
    }

    memset(out, 0, 32);

    size_t byte_len = (len + 1) / 2;
    size_t offset = 32 - byte_len;

    size_t src = 0;

    if (len & 1) {
        char tmp[2] = {
            hex[0],
            '\0'
        };

        char *end = NULL;

        unsigned long v = strtoul(tmp, &end, 16);

        if (*end != '\0') {
            return 0;
        }

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

        if (*end != '\0') {
            return 0;
        }

        out[offset++] = (uint8_t)v;

        src += 2;
    }

    return 1;
}

static int base58_value(char c)
{
    const char *p = strchr(BASE58, c);

    if (p == NULL) {
        return -1;
    }

    return (int)(p - BASE58);
}

/*
 * Decode Base58 para bytes.
 *
 * Para endereço Bitcoin P2PKH esperamos 25 bytes:
 *
 * 1 byte  version
 * 20 bytes HASH160
 * 4 bytes checksum
 */
static int base58_decode(
    const char *input,
    uint8_t *output,
    size_t *output_len
)
{
    size_t input_len = strlen(input);

    size_t zeros = 0;

    while (
        zeros < input_len &&
        input[zeros] == '1'
    ) {
        zeros++;
    }

    size_t size =
        (input_len - zeros) * 733 / 1000 + 1;

    uint8_t *b256 = calloc(size, 1);

    if (!b256) {
        return 0;
    }

    for (size_t i = zeros; i < input_len; i++) {

        int value = base58_value(input[i]);

        if (value < 0) {
            free(b256);
            return 0;
        }

        int carry = value;

        for (
            ssize_t j = (ssize_t)size - 1;
            j >= 0;
            j--
        ) {
            carry += 58 * b256[j];

            b256[j] = carry & 0xff;

            carry >>= 8;
        }

        if (carry != 0) {
            free(b256);
            return 0;
        }
    }

    size_t pos = 0;

    while (
        pos < size &&
        b256[pos] == 0
    ) {
        pos++;
    }

    size_t decoded_len =
        zeros + (size - pos);

    if (decoded_len > *output_len) {
        free(b256);
        return 0;
    }

    memset(output, 0, decoded_len);

    memcpy(
        output + zeros,
        b256 + pos,
        size - pos
    );

    *output_len = decoded_len;

    free(b256);

    return 1;
}

static void sha256d(
    const uint8_t *data,
    size_t len,
    uint8_t out[32]
)
{
    uint8_t tmp[32];

    SHA256(data, len, tmp);
    SHA256(tmp, sizeof(tmp), out);
}

static int p2pkh_address_to_hash160(
    const char *address,
    uint8_t hash160_out[20]
)
{
    uint8_t decoded[64];

    size_t decoded_len = sizeof(decoded);

    if (!base58_decode(
        address,
        decoded,
        &decoded_len
    )) {
        return 0;
    }

    if (decoded_len != 25) {
        fprintf(
            stderr,
            "Endereco Base58Check inesperado: %zu bytes\n",
            decoded_len
        );

        return 0;
    }

    /*
     * P2PKH Bitcoin mainnet:
     * version = 0x00
     */
    if (decoded[0] != 0x00) {
        fprintf(
            stderr,
            "Endereco nao e P2PKH Bitcoin mainnet\n"
        );

        return 0;
    }

    uint8_t checksum[32];

    sha256d(
        decoded,
        21,
        checksum
    );

    if (
        memcmp(
            checksum,
            decoded + 21,
            4
        ) != 0
    ) {
        fprintf(
            stderr,
            "Checksum Base58Check invalido\n"
        );

        return 0;
    }

    memcpy(
        hash160_out,
        decoded + 1,
        20
    );

    return 1;
}

static void hash160(
    const uint8_t *data,
    size_t len,
    uint8_t output[20]
)
{
    uint8_t sha[32];

    SHA256(
        data,
        len,
        sha
    );

    RIPEMD160(
        sha,
        sizeof(sha),
        output
    );
}

int main(int argc, char **argv)
{
    if (argc != 3) {

        fprintf(
            stderr,
            "\nUso:\n"
            "  %s PRIVATE_KEY_HEX ENDERECO_P2PKH\n\n"
            "Exemplo:\n"
            "  %s "
            "0000000000000000000000000000000000000000000000000000000000000001 "
            "1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH\n\n",
            argv[0],
            argv[0]
        );

        return 1;
    }

    const char *private_hex = argv[1];
    const char *target_address = argv[2];

    uint8_t private_key[32];
    uint8_t target_hash160[20];

    /*
     * -------------------------
     * Private key HEX -> 32 bytes
     * -------------------------
     */
    if (!hex_to_32bytes(
        private_hex,
        private_key
    )) {
        fprintf(
            stderr,
            "Private key HEX invalida\n"
        );

        return 1;
    }

    /*
     * -------------------------
     * Endereco -> HASH160
     * -------------------------
     */
    if (!p2pkh_address_to_hash160(
        target_address,
        target_hash160
    )) {
        fprintf(
            stderr,
            "Endereco Bitcoin invalido\n"
        );

        return 1;
    }

    /*
     * -------------------------
     * Inicializa secp256k1
     * -------------------------
     */
    secp256k1_context *ctx =
        secp256k1_context_create(
            SECP256K1_CONTEXT_NONE
        );

    if (!ctx) {
        fprintf(
            stderr,
            "Falha ao criar contexto secp256k1\n"
        );

        return 1;
    }

    /*
     * Valida private key
     */
    if (!secp256k1_ec_seckey_verify(
        ctx,
        private_key
    )) {
        fprintf(
            stderr,
            "Private key fora do intervalo valido secp256k1\n"
        );

        secp256k1_context_destroy(ctx);

        return 1;
    }

    /*
     * -------------------------
     * Private -> Public key
     * -------------------------
     */
    secp256k1_pubkey public_key;

    if (!secp256k1_ec_pubkey_create(
        ctx,
        &public_key,
        private_key
    )) {
        fprintf(
            stderr,
            "Falha ao gerar public key\n"
        );

        secp256k1_context_destroy(ctx);

        return 1;
    }

    /*
     * -------------------------
     * Public key comprimida
     * 33 bytes
     * -------------------------
     */
    uint8_t compressed_pubkey[33];

    size_t compressed_len =
        sizeof(compressed_pubkey);

    if (!secp256k1_ec_pubkey_serialize(
        ctx,
        compressed_pubkey,
        &compressed_len,
        &public_key,
        SECP256K1_EC_COMPRESSED
    )) {
        fprintf(
            stderr,
            "Falha ao serializar public key\n"
        );

        secp256k1_context_destroy(ctx);

        return 1;
    }

    /*
     * -------------------------
     * HASH160(pubkey comprimida)
     * -------------------------
     */
    uint8_t candidate_hash160[20];

    hash160(
        compressed_pubkey,
        compressed_len,
        candidate_hash160
    );

    /*
     * -------------------------
     * Informacoes
     * -------------------------
     */
    printf("\nPrivate key:\n");
    print_hex(
        private_key,
        sizeof(private_key)
    );

    printf("\nPublic key comprimida:\n");
    print_hex(
        compressed_pubkey,
        compressed_len
    );

    printf("\nHASH160 calculado:\n");
    print_hex(
        candidate_hash160,
        sizeof(candidate_hash160)
    );

    printf("\nHASH160 do endereco alvo:\n");
    print_hex(
        target_hash160,
        sizeof(target_hash160)
    );

    printf(
        "\nEndereco alvo:\n%s\n",
        target_address
    );

    /*
     * -------------------------
     * Comparacao final
     * -------------------------
     */
    if (
        memcmp(
            candidate_hash160,
            target_hash160,
            20
        ) == 0
    ) {
        printf(
            "\n========================\n"
            "MATCH\n"
            "========================\n"
        );
    } else {
        printf(
            "\nNO MATCH\n"
        );
    }

    secp256k1_context_destroy(ctx);

    return 0;
}
