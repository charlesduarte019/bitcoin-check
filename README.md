# Bitcoin HASH160 Tools

A small collection of command-line utilities for working with Bitcoin P2PKH addresses, private keys, compressed public keys, and HASH160 values.

The project currently provides three independent tools:

- `priv2hash160` — derives the compressed public key from a Bitcoin private key and calculates its HASH160.
- `addr2hash160` — decodes a Bitcoin P2PKH Base58Check address and extracts its HASH160.
- `keycheck` — verifies whether a specific private key corresponds to a target Bitcoin P2PKH address using a compressed public key.

The tools are written in C with a focus on simplicity, correctness, and performance.

## Features

- Convert a Bitcoin private key in hexadecimal format to a compressed public key.
- Calculate `RIPEMD160(SHA256(public_key))`.
- Decode Bitcoin Base58Check addresses.
- Validate Base58Check checksums.
- Extract HASH160 directly from P2PKH addresses.
- Verify whether a private key corresponds to a Bitcoin P2PKH address.
- Validate Bitcoin mainnet P2PKH addresses.
- Uses `libsecp256k1` for elliptic curve operations.
- Uses OpenSSL for SHA-256 and RIPEMD-160.
- Docker support.
- Minimal runtime dependencies.
- Suitable for command-line tools, testing, interoperability, and educational purposes.

## Available Tools

### `priv2hash160`

Converts a private key into:

```text
Private Key
    │
    ▼
secp256k1
    │
    ▼
Compressed Public Key
    │
    ▼
SHA-256
    │
    ▼
RIPEMD-160
    │
    ▼
HASH160
```

### `addr2hash160`

Extracts the HASH160 embedded in a Bitcoin P2PKH address:

```text
P2PKH Address
    │
    ▼
Base58Check Decode
    │
    ├── Version Byte
    │
    ├── HASH160
    │
    └── Checksum
    │
    ▼
HASH160
```

### `keycheck`

Checks whether a private key corresponds to a target Bitcoin P2PKH address:

```text
                         Target P2PKH Address
                                 │
                                 ▼
                         Base58Check Decode
                                 │
                                 ▼
                          Target HASH160
                                 │
                                 │
Private Key                      │
    │                            │
    ▼                            │
secp256k1                        │
    │                            │
    ▼                            │
Compressed Public Key            │
    │                            │
    ▼                            │
SHA-256                          │
    │                            │
    ▼                            │
RIPEMD-160                       │
    │                            │
    ▼                            │
Candidate HASH160 ───────────────┘
              │
              ▼
           Compare
              │
        ┌─────┴─────┐
        ▼           ▼
      MATCH      NO MATCH
```

The target address is decoded only once, and the final comparison is performed directly between the two 20-byte HASH160 values.

## Bitcoin P2PKH Structure

A standard Bitcoin mainnet P2PKH Base58Check address contains:

```text
+--------------+----------------------+----------+
| Version Byte |       HASH160        | Checksum |
+--------------+----------------------+----------+
|    1 byte    |       20 bytes       | 4 bytes  |
+--------------+----------------------+----------+
```

For Bitcoin mainnet P2PKH addresses, the version byte is:

```text
0x00
```

The checksum is derived from:

```text
SHA256(SHA256(version + HASH160))
```

The first four bytes of the resulting hash are used as the Base58Check checksum.

## Example

Consider the following private key:

```text
0000000000000000000000000000000000000000000000000000000000000001
```

Its compressed public key is:

```text
0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798
```

The resulting HASH160 is:

```text
751e76e8199196d454941c45d1b3a323f1433bd6
```

The corresponding compressed P2PKH Bitcoin address is:

```text
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

The three utilities can therefore be tested using the same known vector.

## Project Structure

```text
.
├── Dockerfile
├── LICENSE
├── README.md
├── addr2hash160.c
├── keycheck.c
└── priv2hash160.c
```

## Requirements

When building directly on the host, the following dependencies are required:

- GCC or compatible C compiler
- OpenSSL development libraries
- libsecp256k1

### Debian / Ubuntu

```bash
sudo apt update

sudo apt install \
    build-essential \
    libsecp256k1-dev \
    libssl-dev
```

## Building From Source

### Build `priv2hash160`

```bash
gcc \
    -O3 \
    -march=native \
    -flto \
    -DNDEBUG \
    priv2hash160.c \
    -lsecp256k1 \
    -lcrypto \
    -o priv2hash160
```

### Build `addr2hash160`

```bash
gcc \
    -O3 \
    -march=native \
    -flto \
    -DNDEBUG \
    addr2hash160.c \
    -lcrypto \
    -o addr2hash160
```

### Build `keycheck`

```bash
gcc \
    -O3 \
    -march=native \
    -flto \
    -DNDEBUG \
    keycheck.c \
    -lsecp256k1 \
    -lcrypto \
    -o keycheck
```

## Usage

### Private Key to HASH160

Run:

```bash
priv2hash160 <PRIVATE_KEY_HEX>
```

Example:

```bash
priv2hash160 \
0000000000000000000000000000000000000000000000000000000000000001
```

Example output:

```text
private_key=0000000000000000000000000000000000000000000000000000000000000001
public_key=0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798
hash160=751e76e8199196d454941c45d1b3a323f1433bd6
```

## P2PKH Address to HASH160

Run:

```bash
addr2hash160 <P2PKH_ADDRESS>
```

Example:

```bash
addr2hash160 \
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

Example output:

```text
address=1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
version=00
hash160=751e76e8199196d454941c45d1b3a323f1433bd6
checksum=<checksum>
```

## Check Private Key Against Address

Run:

```bash
keycheck <PRIVATE_KEY_HEX> <P2PKH_ADDRESS>
```

Example:

```bash
keycheck \
0000000000000000000000000000000000000000000000000000000000000001 \
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

Expected output includes:

```text
Private key:
0000000000000000000000000000000000000000000000000000000000000001

Public key compressed:
0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798

Calculated HASH160:
751e76e8199196d454941c45d1b3a323f1433bd6

Target HASH160:
751e76e8199196d454941c45d1b3a323f1433bd6

Target address:
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH

========================
MATCH
========================
```

If the private key does not correspond to the target address:

```text
NO MATCH
```

## Docker

The Docker image builds and includes all three utilities:

```text
/usr/local/bin/priv2hash160
/usr/local/bin/addr2hash160
/usr/local/bin/keycheck
```

### Dockerfile Example

```dockerfile
FROM debian:bookworm-slim AS build

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        libsecp256k1-dev \
        libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src

COPY priv2hash160.c .
COPY addr2hash160.c .
COPY keycheck.c .

RUN gcc \
    -O3 \
    -march=native \
    -flto \
    -DNDEBUG \
    priv2hash160.c \
    -lsecp256k1 \
    -lcrypto \
    -o priv2hash160

RUN gcc \
    -O3 \
    -march=native \
    -flto \
    -DNDEBUG \
    addr2hash160.c \
    -lcrypto \
    -o addr2hash160

RUN gcc \
    -O3 \
    -march=native \
    -flto \
    -DNDEBUG \
    keycheck.c \
    -lsecp256k1 \
    -lcrypto \
    -o keycheck


FROM debian:bookworm-slim

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        libsecp256k1-1 \
        libssl3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/priv2hash160 /usr/local/bin/
COPY --from=build /src/addr2hash160 /usr/local/bin/
COPY --from=build /src/keycheck /usr/local/bin/

CMD ["/bin/bash"]
```

### Build the Image

```bash
docker build -t bitcoin-hash160-tools .
```

## Running With Docker

### Private Key to HASH160

```bash
docker run --rm \
    bitcoin-hash160-tools \
    priv2hash160 \
    0000000000000000000000000000000000000000000000000000000000000001
```

### Address to HASH160

```bash
docker run --rm \
    bitcoin-hash160-tools \
    addr2hash160 \
    1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

### Check Private Key Against Address

```bash
docker run --rm \
    bitcoin-hash160-tools \
    keycheck \
    0000000000000000000000000000000000000000000000000000000000000001 \
    1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

Because all binaries are installed into `/usr/local/bin`, they can be executed directly without specifying their full path.

For example:

```bash
docker run --rm bitcoin-hash160-tools priv2hash160 1
```

or:

```bash
docker run --rm bitcoin-hash160-tools \
    keycheck \
    1 \
    1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

## HASH160

HASH160 is the composition of two cryptographic hash functions:

```text
HASH160(x) = RIPEMD160(SHA256(x))
```

Bitcoin P2PKH addresses use the HASH160 of a public key.

For compressed public keys:

```text
Private Key
      │
      ▼
  secp256k1
      │
      ▼
Compressed Public Key
    33 bytes
      │
      ▼
    SHA256
      │
      ▼
  RIPEMD160
      │
      ▼
    20 bytes
```

## Compressed Public Keys

Compressed secp256k1 public keys are 33 bytes long.

They start with either:

```text
02
```

or:

```text
03
```

followed by the 32-byte X coordinate.

The prefix identifies which Y coordinate corresponds to the elliptic curve point.

An uncompressed public key is 65 bytes long and begins with:

```text
04
```

This project currently derives HASH160 values from compressed public keys.

## Address Validation

`addr2hash160` and `keycheck` validate the supplied address before using its HASH160.

The validation includes:

1. Validate Base58 characters.
2. Decode the Base58 representation.
3. Verify the decoded payload size.
4. Validate the Base58Check checksum.
5. Verify the Bitcoin mainnet P2PKH version byte (`0x00`).
6. Extract the 20-byte HASH160 payload.

This ensures that malformed or incorrectly checksummed addresses are rejected.

## `keycheck` Comparison Strategy

`keycheck` does not repeatedly convert the candidate public key into a Base58 Bitcoin address.

Instead, the target address is decoded once:

```text
Target Address
      │
      ▼
Base58Check Decode
      │
      ▼
Target HASH160
```

The candidate private key is processed as:

```text
Private Key
      │
      ▼
secp256k1
      │
      ▼
Compressed Public Key
      │
      ▼
SHA256
      │
      ▼
RIPEMD160
      │
      ▼
Candidate HASH160
```

The final operation is effectively:

```c
memcmp(candidate_hash160, target_hash160, 20)
```

This avoids unnecessary Base58 encoding when only equality needs to be determined.

## Performance

The project is compiled with:

```text
-O3
-march=native
-flto
-DNDEBUG
```

Elliptic curve operations are handled by `libsecp256k1`, an optimized implementation of the secp256k1 elliptic curve used by Bitcoin.

Cryptographic hash operations are provided by OpenSSL.

For private-key processing, the most computationally expensive operation is generally:

```text
Private Key
      ↓
secp256k1 scalar multiplication
      ↓
Public Key
```

SHA-256, RIPEMD-160, and the final 20-byte comparison are comparatively inexpensive.

## Docker Design

The Dockerfile uses a multi-stage build.

The build stage contains:

- compiler;
- headers;
- development libraries;
- source files.

The final runtime image only contains the runtime libraries and the compiled binaries.

The resulting filesystem includes:

```text
/usr/local/bin/
├── addr2hash160
├── keycheck
└── priv2hash160
```

This keeps the runtime image smaller and avoids shipping the compiler and development packages.

## Testing

A known Bitcoin test vector can be used to verify all three tools.

Private key:

```text
0000000000000000000000000000000000000000000000000000000000000001
```

Compressed public key:

```text
0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798
```

HASH160:

```text
751e76e8199196d454941c45d1b3a323f1433bd6
```

P2PKH address:

```text
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

### Test `priv2hash160`

```bash
priv2hash160 \
0000000000000000000000000000000000000000000000000000000000000001
```

Expected HASH160:

```text
751e76e8199196d454941c45d1b3a323f1433bd6
```

### Test `addr2hash160`

```bash
addr2hash160 \
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

Expected HASH160:

```text
751e76e8199196d454941c45d1b3a323f1433bd6
```

### Test `keycheck`

```bash
keycheck \
0000000000000000000000000000000000000000000000000000000000000001 \
1BgGZ9tcN4rm9KBzDn7KprQz87SZ26SAMH
```

Expected result:

```text
MATCH
```

A different private key should return:

```text
NO MATCH
```

## Security Notes

Private keys control Bitcoin funds.

Avoid passing valuable private keys as command-line arguments on shared or untrusted systems because command-line arguments may be visible through:

- process inspection;
- shell history;
- monitoring software;
- container logs;
- system auditing;
- CI/CD logs.

For sensitive workloads, consider changing the tools to read private keys from:

- standard input;
- protected files;
- dedicated secret stores;
- offline systems.

Never publish, log, commit, or share private keys containing real funds.

## Scope

This project is intentionally small and currently focuses on three operations:

```text
Private Key
    ↓
Compressed Public Key
    ↓
HASH160
```

```text
P2PKH Base58Check Address
    ↓
HASH160
```

and:

```text
Private Key + P2PKH Address
            ↓
        HASH160
            ↓
         Compare
```

It does not currently implement:

- Wallet management
- WIF private key decoding
- HD wallets
- BIP32
- BIP39
- BIP44
- P2SH
- Bech32
- SegWit
- Taproot
- Transaction signing
- Blockchain communication
- Network synchronization

## Contributing

Contributions are welcome.

If you would like to improve the project:

1. Fork the repository.
2. Create a feature branch.

```bash
git checkout -b feature/my-feature
```

3. Commit your changes.

```bash
git commit -m "Add my feature"
```

4. Push the branch.

```bash
git push origin feature/my-feature
```

5. Open a Pull Request.

When contributing, please keep the implementation:

- simple;
- portable where practical;
- well documented;
- memory safe;
- focused on correctness;
- compatible with existing command-line behavior.

## Reporting Issues

When reporting a bug, include:

- Operating system
- CPU architecture
- Compiler version
- libsecp256k1 version
- OpenSSL version
- Docker version, if applicable
- Command executed
- Expected behavior
- Actual behavior

Do not include real or sensitive private keys in issue reports.

## Disclaimer

This software is provided for educational, development, interoperability, and cryptographic research purposes.

Users are responsible for ensuring that their use of this software complies with applicable laws and that they only process cryptographic material they are authorized to use.

The authors and contributors are not responsible for lost funds, exposed private keys, misuse, or damages resulting from the use of this software.

## License

This project is licensed under the **MIT License**.
