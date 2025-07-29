## Raspberry-Pi-Pico-W-IoT-Control-System-with-MQTT

This project implements a comprehensive IoT control system using the Raspberry Pi Pico W microcontroller with WiFi connectivity. The system provides bidirectional communication through MQTT protocol, enabling real-time monitoring of physical inputs (buttons and joystick) and remote control of RGB LEDs via cloud-based MQTT broker, which uses the HiveMQ Broker.


![20250729_081249](https://github.com/user-attachments/assets/27dd95d9-e49c-490c-a96d-89ef274638fb)


Key Features
*Raspberry Pi Pico W - Main microcontroller with built-in WiFi
*Input Monitoring: 2 push buttons + 1 joystick button
*RGB LED Control: Independent control of Red, Green, and Blue LEDs
*GPIO Configuration: Pull-up resistors for inputs, direct GPIO control for outputs

The project aims to monitor the status of buttons and joysticks in real time every second on the Broker, where we also monitor whether the microcontroller is active/alive every 30 seconds.

*Protocols/Connectivity used:
*Wi-Fi Connection
*MQTT Protocol with access port 1883
*DNS Resolution

## 📦 Installation
1. Clone this repository:
```sh
git clone https://github.com/yourusername/yourrepository.git
cd yourrepository
```
2. Configure the environment and compile using Pico SDK and CMake:
```sh
mkdir build
cd build
cmake ..
ninja
```
3. Upload the `.uf2` to the Pico W.

## 📝 Usage
- Configure WiFi and MQTT credentials in `src/config.h`
- Run and monitor the status via MQTT Dashboard

## 🧩 Project Architecture
```
src/
main.c
mqtt_client.c
gpio_handler.c
...
docs/
tabela.png
...
```

## ❗ Requirements
- Raspberry Pi Pico W
- Public MQTT broker (e.g., mqtt-dashboard.com)
- CMake, Ninja, Pico SDK

🔧 Hardware Configuration
Pinout
📍 ENTRADAS (Pull-up habilitado):
├── Pino 5  → Botão A
├── Pino 6  → Botão B
└── Pino 22 → Joystick

🌈 SAÍDAS (LEDs RGB):
├── Pino 13 → LED Vermelho
├── Pino 11 → LED Verde
└── Pino 12 → LED Azul


MQTT Topic Structure                                                          
pico/button_a      → Button A state (TRUE/FALSE)                               
pico/button_b      → Button B state (TRUE/FALSE)                              
pico/joystick      → Joystick state (TRUE/FALSE)                             
pico/heartbeat     → System health status                                    
pico/status        → System startup messages                                 
pico/led_command   → LED control commands (subscribed)                       
                                                                                                                                                

Command Interface
LED_RED:ON         → Turn red LED on
LED_RED:OFF        → Turn red LED off
LED_GREEN:ON       → Turn green LED on
LED_GREEN:OFF      → Turn green LED off
LED_BLUE:ON        → Turn blue LED on
LED_BLUE:OFF       → Turn blue LED off
LED_ALL_ON         → Turn all LEDs on
LED_ALL_OFF        → Turn all LEDs off


Programming Framework
-Language: C/C++
-SDK: Raspberry Pi Pico SDK 1.5.1
-Network Stack: lwIP (Lightweight IP)
-TLS Library: mbedTLS (not currently used)
-Build System: CMake with Ninja generator


Conclusion
This project demonstrates a complete IoT solution combining embedded hardware control with cloud-based communication. The system provides reliable, real-time bidirectional communication suitable for various IoT applications, from simple remote monitoring to complex automation systems. The modular architecture allows for easy extension and customization for specific use cases.


## 🤝 How to Contribute
1. Make a Fork
2. Create a Branch for Your Feature (`git checkout -b my-feature`)
3. Commit Your Changes (`git commit -am 'New Feature'`)
4. Push to the Branch (`git push origin my-feature`)
5. Open a Pull Request

## 📄 License
MIT - See the [LICENSE](LICENSE) file for details.
