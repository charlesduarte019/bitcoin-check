#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/sha.h>

static const char *BASE58 =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static void print_hex(
    const uint8_t *data,
    size_t len
)
{
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }

    printf("\n");
}

static int base58_value(char c)
{
    const char *p = strchr(BASE58, c);

    if (p == NULL)
        return -1;

    return (int)(p - BASE58);
}

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

    uint8_t *b256 =
        calloc(size, 1);

    if (!b256)
        return 0;

    for (
        size_t i = zeros;
        i < input_len;
        i++
    ) {
        int value =
            base58_value(input[i]);

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
            carry +=
                58 * b256[j];

            b256[j] =
                carry & 0xff;

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

    memset(
        output,
        0,
        decoded_len
    );

    memcpy(
        output + zeros,
        b256 + pos,
        size - pos
    );

    *output_len =
        decoded_len;

    free(b256);

    return 1;
}

static void sha256d(
    const uint8_t *data,
    size_t len,
    uint8_t output[32]
)
{
    uint8_t tmp[32];

    SHA256(
        data,
        len,
        tmp
    );

    SHA256(
        tmp,
        sizeof(tmp),
        output
    );
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(
            stderr,
            "Uso:\n"
            "  %s ENDERECO_P2PKH\n",
            argv[0]
        );

        return 1;
    }

    const char *address =
        argv[1];

    uint8_t decoded[64];

    size_t decoded_len =
        sizeof(decoded);

    if (!base58_decode(
        address,
        decoded,
        &decoded_len
    )) {
        fprintf(stderr, "Base58 invalido\n");
        return 1;
    }

    if (decoded_len != 25) {
        fprintf(
            stderr,
            "Endereco inesperado: %zu bytes\n",
            decoded_len
        );

        return 1;
    }

    /*
     * Mainnet P2PKH
     */
    if (decoded[0] != 0x00) {
        fprintf(
            stderr,
            "Nao e endereco P2PKH mainnet\n"
        );

        return 1;
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
            "Checksum invalido\n"
        );

        return 1;
    }

    printf(
        "address=%s\n",
        address
    );

    printf("version=%02x\n", decoded[0]);

    printf("hash160=");
    print_hex(
        decoded + 1,
        20
    );

    printf("checksum=");
    print_hex(
        decoded + 21,
        4
    );

    return 0;
}
