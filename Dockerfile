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
