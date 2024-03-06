# METAR Flight Category Lamp

This project involves creating a lamp that displays the flight category from METAR (Meteorological Aerodrome Report) data. The lamp changes its color based on the flight category obtained from the METAR data.

## Features

- Retrieves METAR data for specified airports
- Parses METAR data to determine flight category
- Changes lamp color based on flight category (VFR, MVFR, IFR, LIFR)
- Wi-Fi connectivity for internet access
- LED lamp for visual indication

## Setup

### Requirements

- Microcontroller board (Arduino, ESP8266, etc.)
- Wi-Fi module compatible with the microcontroller
- RGB LED or LED strip for lamp
- Connection to the internet

### Installation

1. Connect the necessary components to the microcontroller board.
2. Upload the provided source code to the microcontroller.
3. Ensure proper configuration of Wi-Fi credentials in the code.
4. Compile and upload the code to the microcontroller.

## Usage

1. Upon power-up, the microcontroller initializes and connects to the configured Wi-Fi network.
2. If the reprogram button is pressed, the microcontroller enters reprogram mode.
3. Otherwise, it connects to the access point and retrieves METAR data for specified airports.
4. The retrieved METAR data is parsed to determine the flight category (VFR, MVFR, IFR, LIFR).
5. The lamp changes its color accordingly:
   - VFR: Green
   - MVFR: Blue
   - IFR: Red
   - LIFR: Magenta
6. LED indicators provide status updates.

## Contributors

- [Your Name/Username]

## License

This project is licensed under the [License Name] License - see the LICENSE.md file for details.

## Acknowledgments

- [Mention any external libraries or resources used]
