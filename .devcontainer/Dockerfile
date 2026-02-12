# =============================================================================
# Dev container: Full development environment with OpenFHE + Drogon pre-built
# =============================================================================
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# ── Build tools + runtime deps ───────────────────────────────────────────────
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build essentials
    build-essential \
    cmake \
    git \
    pkg-config \
    gdb \
    valgrind \
    clang-format \
    clang-tidy \
    # OpenMP
    libomp-dev \
    # Drogon deps
    libjsoncpp-dev \
    libssl-dev \
    zlib1g-dev \
    uuid-dev \
    libc-ares-dev \
    libbrotli-dev \
    libhiredis-dev \
    libyaml-cpp-dev \
    # Utilities
    curl \
    wget \
    sudo \
    vim \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# ── Build OpenFHE ────────────────────────────────────────────────────────────
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

# ── Build Drogon ─────────────────────────────────────────────────────────────
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

RUN ldconfig

# ── Non-root dev user ────────────────────────────────────────────────────────
ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME -s /bin/bash \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/$USERNAME

USER $USERNAME
WORKDIR /workspace
