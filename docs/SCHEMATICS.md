# USB2Saturn Schematics and Wiring

This document describes how to connect the Waveshare RP2350-USB-A development board to a Sega Saturn controller cable.

## Connector Physical Layout

The Sega Saturn uses a 9-pin connector arranged in a single row with beveled edges at the top. 

### 1. Console Port (Female)
If you are looking **directly at the front of the Sega Saturn console** into the port:
```text
      (Beveled Edge)
  _______________________
 /                       \
|  1  2  3  4  5  6  7  8  9  |
 -------------------------
```

### 2. Controller Cable Plug (Male)
If you cut a controller cable and look **directly into the male plug** (the pins facing you):
```text
      (Beveled Edge)
  _______________________
 /                       \
|  9  8  7  6  5  4  3  2  1  |
 -------------------------
```
*Note: If you look into the female port on the console, Pin 1 is on the left and Pin 9 is on the right.*

### 3. Waveshare RP2350-USB-A Board

The RP2350-USB-A board is shaped like a USB flash drive with castellated holes along its sides. The exact pin labels are printed on the silkscreen on the back of the board. A generic representation of the layout looks like this:

```text
         [ USB-A Plug ]
        ________________
       |                |
  5V --| 5V         GP0 |-- GP0  (S0 / TH)
 GND --| GND        GP1 |-- GP1  (S1 / TR)
 3V3 --| 3V3        GP2 |-- GP2  (D0)
GP29 --| GP29       GP3 |-- GP3  (D1)
GP28 --| GP28       GP4 |-- GP4  (D2)
GP27 --| GP27       GP5 |-- GP5  (D3)
GP26 --| GP26       GP6 |-- GP6
GP10 --| GP10       GP7 |-- GP7
 GP9 --| GP9        GP8 |-- GP8
       |________________|
```
*(Always verify the pin numbers using the printed text on the back of your specific board revision!)*

> [!WARNING]
> **Do not rely on wire colors!**
> There is no universal standard for Sega Saturn controller wire colors. Official controllers from different regions and third-party controllers all use completely different colored wires internally. 
> 
> You MUST use a multimeter in continuity mode to trace each physical pin on the connector to its corresponding wire inside the cable before soldering it to the RP2350. Guessing wire colors can result in sending 5V to the wrong pins and permanently damaging the RP2350 or your console.

## Hardware Protection Best Practices

When connecting a modern microcontroller to a retro console, it is highly recommended to add protective components to safeguard the console's internal hardware from surges, shorts, or back-powering.

### 1. Back-Powering Protection (Crucial)
The Sega Saturn outputs 5V (Pin 1) to power the controller. However, the RP2350-USB-A is designed to be powered via its USB port. **If you plug the RP2350 into a PC/Wall Adapter while it is also plugged into the Sega Saturn, the two 5V power supplies will clash.** This can destroy the Sega Saturn's power rail.
* **Solution**: Place a **Schottky diode** (e.g., `1N5817` or `1N5819`) in series on the 5V line coming from the Saturn. 
  * Connect the *Anode* (non-striped side) to the Saturn's Pin 1 (+5V).
  * Connect the *Cathode* (striped side) to the RP2350's `5V` pin.
  * This allows the Saturn to power the board, but blocks power from flowing *backward* into the Saturn if the USB is plugged in.

### 2. Current Limiting Resistors (Recommended)
If the RP2350 accidentally sets its pins to output 5V/3.3V while the Saturn is simultaneously driving the same line low, a short circuit occurs which can burn out the Saturn's I/O chip.
* **Solution**: Add small series resistors (typically **100Ω to 330Ω**) on every data line (`S0`, `S1`, `D0`-`D3`). This restricts the maximum current to a safe level (around 10mA) even if a short circuit occurs, protecting both devices.

### 3. Logic Level Shifters (Optional but Safest)
While the RP2350 GPIOs are 5V-tolerant *when powered*, they can be damaged if a 5V signal arrives while the board is unpowered. Furthermore, driving a 3.3V signal into a 5V system (the Saturn) usually works, but isn't strictly to 5V TTL spec.
* **Solution**: Use a bi-directional logic level shifter (like the `TXS0108E` module). Connect the 3.3V side to the RP2350 and the 5V side to the Sega Saturn. This perfectly translates the voltages and adds an extra layer of electrical buffering.


## Pinout Mapping

| Sega Saturn Pin | Function | Direction | RP2350-USB-A Pin | Description |
| :--- | :--- | :--- | :--- | :--- |
| 1 | VCC (+5V) | Power Out | VBUS / 5V | Powers the RP2350 from the Saturn. |
| 2 | D1 | Data Out | GPIO 3 | Data bit 1 |
| 3 | D0 | Data Out | GPIO 2 | Data bit 0 |
| 4 | S1 (TR) | Data In | GPIO 1 | Select bit 1 |
| 5 | S0 (TH) | Data In | GPIO 0 | Select bit 0 |
| 6 | Detect (+5V)| Data Out | *Optional* | Often tied to VCC in standard controllers. |
| 7 | D3 | Data Out | GPIO 5 | Data bit 3 |
| 8 | D2 | Data Out | GPIO 4 | Data bit 2 |
| 9 | GND | Power | GND | Common Ground |

> [!IMPORTANT]
> The RP2350 GPIOs are 5V tolerant, meaning we can connect the S0 and S1 pins directly to the 5V signals from the Sega Saturn. The 3.3V logic outputs from D0-D3 will register as Logic HIGH on the 5V Sega Saturn side.
> 
> **However, you MUST ensure that the RP2350 board is powered (via the Saturn's VCC line or USB) BEFORE the Saturn console starts sending 5V logic signals to the GPIOs.** Applying 5V to the GPIO pins while the RP2350 is unpowered can damage the chip. Connecting VCC (Pin 1) to VBUS and Ground (Pin 9) to GND guarantees that the board is powered simultaneously with the signals.

## Wiring Diagram

```mermaid
graph LR
    subgraph Sega Saturn Controller Plug
        P1[Pin 1: +5V VCC]
        P2[Pin 2: D1]
        P3[Pin 3: D0]
        P4[Pin 4: S1 / TR]
        P5[Pin 5: S0 / TH]
        P6[Pin 6: Detect / +5V]
        P7[Pin 7: D3]
        P8[Pin 8: D2]
        P9[Pin 9: GND]
    end

    subgraph RP2350-USB-A
        VBUS[VBUS / 5V In]
        GND_RP[GND]
        GP0[GP0]
        GP1[GP1]
        GP2[GP2]
        GP3[GP3]
        GP4[GP4]
        GP5[GP5]
    end

    P1 --- VBUS
    P9 --- GND_RP
    
    P5 -.->|Input to RP2350| GP0
    P4 -.->|Input to RP2350| GP1
    
    GP2 ===|Output to Saturn| P3
    GP3 ===|Output to Saturn| P2
    GP4 ===|Output to Saturn| P8
    GP5 ===|Output to Saturn| P7
    
    P6 --- P1
```
