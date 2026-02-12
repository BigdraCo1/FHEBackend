# =============================================================================
# Stage 1: Build dependencies (OpenFHE + Drogon) — cached separately
# =============================================================================
FROM ubuntu:24.04 AS deps

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and shared dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    pkg-config \
    libomp-dev \
    # Drogon dependencies
    libjsoncpp-dev \
    libssl-dev \
    zlib1g-dev \
    uuid-dev \
    libc-ares-dev \
    libbrotli-dev \
    # Redis support for Drogon
    libhiredis-dev \
    # Yaml support for Drogon
    libyaml-cpp-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Build and install OpenFHE
ARG OPENFHE_VERSION=v1.4.2
RUN git clone --depth 1 --branch ${OPENFHE_VERSION} \
      https://github.com/openfheorg/openfhe-development.git /tmp/openfhe && \
    cmake -S /tmp/openfhe -B /tmp/openfhe/build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_BENCHMARKS=OFF \
      -DBUILD_UNITTESTS=OFF && \
    cmake --build /tmp/openfhe/build -j"$(nproc)" && \
    cmake --install /tmp/openfhe/build && \
    rm -rf /tmp/openfhe

# Build and install Drogon
ARG DROGON_VERSION=v1.9.12
RUN git clone --depth 1 --branch ${DROGON_VERSION} --recurse-submodules \
      https://github.com/drogonframework/drogon.git /tmp/drogon && \
    cmake -S /tmp/drogon -B /tmp/drogon/build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_CTL=ON \
      -DBUILD_ORM=OFF && \
    cmake --build /tmp/drogon/build -j"$(nproc)" && \
    cmake --install /tmp/drogon/build && \
    rm -rf /tmp/drogon

# Refresh shared library cache
RUN ldconfig

# =============================================================================
# Stage 2: Build the application
# =============================================================================
FROM deps AS builder

WORKDIR /app
COPY . .

RUN cmake -S . -B build \
      -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build -j"$(nproc)"

# =============================================================================
# Stage 3: Minimal runtime image
# =============================================================================
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# Install only runtime libraries (no dev headers, no compilers)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libomp5 \
    libjsoncpp25 \
    libssl3t64 \
    zlib1g \
    libuuid1 \
    libc-ares2 \
    libbrotli1 \
    libhiredis1.1.0 \
    libyaml-cpp0.8 \
    libstdc++6 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Copy the installed libraries from the deps stage
COPY --from=deps /usr/local/lib/ /usr/local/lib/
COPY --from=deps /usr/local/include/ /usr/local/include/
RUN ldconfig

# Copy the built binary and config
WORKDIR /app
COPY --from=builder /app/build/FHEbackend .
COPY --from=builder /app/config.docker.yaml ./config.yaml

# Create uploads directory
RUN mkdir -p /app/uploads

# Expose the application port
EXPOSE 5670

CMD ["./FHEbackend"]
