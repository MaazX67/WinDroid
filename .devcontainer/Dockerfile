FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    libzip-dev \
    mingw-w64 \
    ninja-build \
    pkg-config \
    unzip \
    zip \
    && rm -rf /var/lib/apt/lists/*