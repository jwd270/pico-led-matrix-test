# LED Matrix Test Program

A test program for a 32x64 RGB LED matrix panel using a Raspberry Pi Pico 2 microcontroller.

## Hardware

- **Microcontroller**: Raspberry Pi Pico 2
- **Display**: 32 rows × 64 columns RGB LED matrix panel (HUB75-style interface variant)
- **Power**: 4V supply to LED panel (reduced from 5V due to 3.3V GPIO logic levels)

## Display Architecture

This panel uses a unique HUB75 variant with the following characteristics:

- **Address Lines**: 5-bit addressing (A, B, C, D, E)
  - ADDR_A-D (4 bits): Select row within half (0-15)
  - ADDR_E (1 bit): Half-select signal (0 = top half rows 0-15, 1 = bottom half rows 16-31)
- **RGB Channels**: RGB1 only (drives all 32 rows)
- **Data Shift**: 64 pixels per row, clocked serially
- **Multiplexing**: Time-multiplexed, refreshed at ~100Hz

### Pin Connections

| Signal | GPIO Pin | Description |
|--------|----------|-------------|
| R1     | 2        | Red data |
| G1     | 3        | Green data |
| B1     | 4        | Blue data |
| A      | 10       | Address bit 0 |
| B      | 16       | Address bit 1 |
| C      | 18       | Address bit 2 |
| D      | 20       | Address bit 3 |
| E      | 22       | Half-select (0=top, 1=bottom) |
| CLK    | 11       | Shift clock |
| LAT    | 12       | Latch (output enable for shift registers) |
| OE     | 13       | Output enable (active low) |

## Test Routine

The program runs a continuous two-part test sequence:

### Part 1: Full-Screen Color Test (~8 seconds)

Displays solid colors across the entire panel to verify basic functionality:
- RED (2 seconds)
- GREEN (2 seconds)
- BLUE (2 seconds)
- WHITE (2 seconds)

### Part 2: Row-by-Row Test (~48 seconds)

Tests each row individually by lighting up one full row at a time:
- Cycles through all 32 rows (0-31)
- For each row, displays: RED → GREEN → BLUE
- Each color shown for 500ms
- Total: 32 rows × 3 colors × 0.5s = 48 seconds

### Continuous Loop

After completing both parts, the routine pauses for 1 second and then repeats indefinitely.

**Total cycle time**: ~57 seconds per iteration

## How It Works

### Display Refresh

The display uses time-division multiplexing:

1. **Disable output** (OE = HIGH)
2. **Set row address** (ADDR_A-E select which row)
3. **Shift in 64 pixels** of RGB data for that row (clock in via CLK)
4. **Latch data** (pulse LAT to transfer shift register to output)
5. **Enable output** (OE = LOW) for brief period
6. **Repeat** for all 32 rows

The refresh loop runs continuously at ~100Hz to create a stable image through persistence of vision.

### Timing Characteristics

- **Clock period**: 3μs per bit (1μs data setup, 1μs high, 1μs low)
- **Latch pulse**: 1μs
- **Row display time**: 100μs
- **Refresh rate**: ~100Hz (32 rows × ~310μs per row)

## Building and Running

1. Set up the Pico SDK environment
2. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```
3. Flash `ledmatrix_test.uf2` to your Pico 2
4. Connect to serial console (115200 baud) to see test progress
5. Observe the LED panel cycling through the test patterns

## Notes

- The panel requires a 4V supply due to 3.3V GPIO logic levels not reliably driving 5V logic inputs
- Add level shifter to enable 5v operation.
- The ADDR_E pin acts as a half-select rather than a standard 5th address bit
- RGB2 pins are not used in this panel configuration
