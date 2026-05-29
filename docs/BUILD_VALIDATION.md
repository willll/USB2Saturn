# Compilation and Validation Guide

This document describes the recommended steps to compile USB2Saturn and validate the build outputs.

## 1. Docker Build (Recommended)

Use the project script to build with a known container toolchain.

```bash
./build_with_docker.sh
```

Expected result:

- Docker image `usb2saturn-builder` is created or reused.
- Firmware build completes in `/workspace/build` inside the container.
- Output files are available in the host project `build/` directory:
  - `build/USB2Saturn.uf2`
  - `build/USB2Saturn.elf`

Verify artifacts:

```bash
ls -lh build/USB2Saturn.uf2 build/USB2Saturn.elf
```

## 2. Native Build (Optional)

Use this when building directly on your host machine.

Prerequisites:

- `cmake`
- `arm-none-eabi-gcc`
- A generator/build tool (for example `make` or `ninja`)

Build commands:

```bash
cmake -S . -B build
cmake --build build -j
```

If the existing build directory was generated elsewhere (for example in Docker), create a fresh one:

```bash
cmake -S . -B build_native
cmake --build build_native -j
```

## 3. Host Unit Tests

The project includes host-runnable unit tests for input mapping in `tests/test_input_mapper.c`.

Quick run with gcc:

```bash
gcc -std=c11 -Wall -Wextra -Isrc tests/test_input_mapper.c src/input_mapper.c -o /tmp/usb2saturn_input_mapper_tests
/tmp/usb2saturn_input_mapper_tests
```

Expected output:

- `All input mapper tests passed.`

## 4. CTest Integration (Host Builds)

For non-cross host configurations, CMake defines `usb2saturn_input_mapper_tests` and registers it with CTest.

Example:

```bash
cmake -S . -B build_host -DUSB2SATURN_ENABLE_HOST_TESTS=ON
cmake --build build_host -j
ctest --test-dir build_host --output-on-failure
```

Note:

- The CTest target is disabled automatically for cross-compiling firmware toolchains.

## 5. Firmware Validation Checklist

After successful compilation:

1. Confirm artifact timestamps were updated:
   - `build/USB2Saturn.uf2`
   - `build/USB2Saturn.elf`
2. Flash `USB2Saturn.uf2` to the RP2350 board.
3. Connect a known USB boot mouse and verify:
   - Movement maps to Saturn D-pad directions.
   - Left/right/middle mouse buttons map to Saturn A/B/C.
4. Connect a USB boot keyboard and verify:
  - Arrow keys or WASD map to Saturn directions.
  - Z/X/C map to Saturn A/B/C and Enter or Space maps to Start.
5. Unplug the USB device and verify no stuck button behavior.
6. Connect a generic gamepad and verify fallback input mapping still responds.

## 6. Troubleshooting

Common issues:

- `arm-none-eabi-gcc not found`:
  - Install ARM GCC toolchain or use Docker build.
- `CMakeCache.txt directory/source mismatch`:
  - Build in a fresh output directory (`build_native`, `build_host`, etc.).
- Docker outputs owned by root:
  - If needed, run `sudo chown -R $USER:$USER build` after container builds.
