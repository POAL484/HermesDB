FROM ubuntu:26.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    cmake \
    g++ \
    git \
    libjsoncpp-dev \
    liblz4-dev \
    libsqlite3-dev \
    libssl-dev \
    make \
    uuid-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

RUN git clone --depth 1 https://github.com/drogonframework/drogon.git /tmp/drogon \
    && git -C /tmp/drogon submodule update --init \
    && cmake -S /tmp/drogon -B /tmp/drogon/build \
        -DBUILD_TESTING=OFF \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /tmp/drogon/build --parallel \
    && cmake --install /tmp/drogon/build \
    && git clone --depth 1 https://github.com/google/snappy.git /tmp/snappy \
    && git -C /tmp/snappy submodule update --init \
    && cmake -S /tmp/snappy -B /tmp/snappy/build \
        -DBUILD_TESTING=OFF \
        -DCMAKE_BUILD_TYPE=Release \
    && cmake --build /tmp/snappy/build --parallel \
    && cmake --install /tmp/snappy/build \
    && rm -rf /tmp/drogon /tmp/snappy

WORKDIR /src
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --parallel --target cache_service \
    && install -D build/cache_service /runtime/app/build/cache_service \
    && install -D config.json /runtime/app/config.json \
    && mkdir -p /runtime/usr/share/hermes \
    && ldd build/cache_service \
        | awk '{ for (i = 1; i <= NF; i++) if ($i ~ /^\//) { print $i; break } }' \
        | sort -u \
        | xargs -r -I '{}' cp -L --parents '{}' /runtime

FROM scratch AS runtime

COPY --from=builder /runtime /

WORKDIR /app/build

EXPOSE 80

CMD ["/app/build/cache_service"]
