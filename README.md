# USB2Saturn

A project to use USB input devices (Gamepads, Keyboards, Mice) on a Sega Saturn console using the Waveshare RP2350-USB-A development board.

## Hardware Requirements
- [Waveshare RP2350-USB-A](https://www.waveshare.com/wiki/RP2350-USB-A)
- Sega Saturn Controller Cable (or extension cord to cut)

## Schematics
See [docs/SCHEMATICS.md](docs/SCHEMATICS.md) for the wiring guide.

Board design package:
- [docs/BOARD_DESIGN.md](docs/BOARD_DESIGN.md)
- [docs/BOARD_BOM.csv](docs/BOARD_BOM.csv)
- [docs/BOARD_NETLIST.csv](docs/BOARD_NETLIST.csv)

KiCad skeleton project:
- [hardware/USB2Saturn_Protection/USB2Saturn_Protection.kicad_pro](hardware/USB2Saturn_Protection/USB2Saturn_Protection.kicad_pro)
- [hardware/USB2Saturn_Protection/README.md](hardware/USB2Saturn_Protection/README.md)

Pinout data sources:
- Sega Saturn controller pinout: https://gamesx.com/controldata/saturn.htm
- Waveshare RP2350-USB-A pin reference: https://www.waveshare.com/wiki/RP2350-USB-A?srsltid=AfmBOord3EtosYRN9eA4ZmPHfdGHGz5l1G7hL_v2CVy890FmMrs2h8b_

## Building the Project
This project uses the Raspberry Pi Pico C/C++ SDK. 

### Prerequisites
- CMake
- ARM GCC Toolchain (`arm-none-eabi-gcc`)
- Pico SDK (included as a submodule)

### Build Instructions
```bash
git clone --recursive https://github.com/YOUR_USERNAME/USB2Saturn.git
cd USB2Saturn
mkdir build
cd build
cmake ..
make
```

### Flashing
1. Hold the `BOOT` button on the RP2350-USB-A board and plug it into your PC.
2. It will mount as a mass storage device.
3. Drag and drop the `USB2Saturn.uf2` file from the `build` directory onto the drive.
4. The board will automatically reboot and start running the software.

## Current Status
- Supports generic HID Gamepads mapped to the standard Sega Saturn controller layout.
- The Sega Saturn bit-bang protocol is implemented using GPIO interrupts.
- Currently Keyboard and Mouse HID reports are stubbed but not fully mapped.

## License
MIT License
