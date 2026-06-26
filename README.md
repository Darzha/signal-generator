# Signal Generator

Arduino-based DDS function generator using the AD9833 chip, featuring a joystick-controlled menu system and LCD display.
Active noise cancellation is not equally effective across all frequencies. This project was built to generate controlled test signals for exploring how different frequencies interact with ANC systems. One potential application is investigating warning tones or safety alerts that remain more perceptible to people wearing ANC headphones, helping improve awareness in situations such as pedestrian or workplace safety.

## Features

- Sine, triangle, and square wave output
- Frequency range: 1 Hz to 2 MHz
- Frequency adjustment with decade multiplier (1x, 10x, 100x, etc.)
- LCD contrast and backlight brightness control
- Output on/off toggle via joystick button
- Smooth joystick navigation with acceleration

## Hardware Required

| Component | Notes |
|-----------|-------|
| Arduino Nano or Uno | |
| AD9833 DDS module | SPI interface |
| 16x2 LCD | HD44780 compatible, 4-bit mode |
| Analog joystick | KY-023 or similar, with push button |
| 1kΩ resistor | For contrast RC filter |
| 47-100µF capacitor | For contrast RC filter |
| Breadboard and jumper wires | |

## Wiring

### LCD (4-bit mode)
| LCD Pin | Arduino Pin |
|---------|-------------|
| RS | 2 |
| E | 8 |
| D4 | 4 |
| D5 | 5 |
| D6 | 6 |
| D7 | 7 |
| V0 | 1kΩ resistor + capacitor to GND, PWM from pin 9 |
| LED+ | Pin 3 (PWM) |
| LED- | GND |
| VSS | GND |
| VDD | 5V |
| RW | GND |

### AD9833 DDS
| DDS Pin | Arduino Pin |
|---------|-------------|
| FSYNC | 10 |
| SDATA | 11 (MOSI) |
| SCLK | 13 (SCK) |
| VCC | 5V |
| AGND/DGND | GND |

### Joystick
| Joystick Pin | Arduino Pin |
|--------------|-------------|
| VRx | A0 |
| VRy | A1 |
| SW | A2 |
| VCC | 5V |
| GND | GND |

## Usage

- Move joystick **up/down** to navigate between menus
- Move joystick **left/right** to adjust values
- Push joystick button to toggle output on/off
- A filled square (█) on the top-right means output is ON
- An empty square (□) means output is OFF

## Menus

1. **Contrast** — LCD contrast adjustment
2. **Light** — LCD backlight brightness
3. **Shape** — Waveform: Square, Triangle, Sine
4. **Frequency** — Set frequency multiplier (10-999)
5. **Decade** — Set decade multiplier (1, 10, 100, 1000, 10000)

## License

MIT
