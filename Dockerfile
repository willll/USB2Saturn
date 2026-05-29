FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary tools for Pico SDK development
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    python3 \
    git \
    && rm -rf /var/lib/apt/lists/*

# Set up the workspace
WORKDIR /workspace

# Default command compiles the project
CMD ["bash", "-c", "mkdir -p build && cd build && cmake .. && make -j$(nproc)"]
