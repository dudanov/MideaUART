# MideaUART
Arduino framework library for controlling Midea home appliances using the UART protocol.

Control is possible with a custom dongle. Example of hardware implementation is [IoT Uni Dongle](https://github.com/dudanov/iot-uni-dongle).

## Using
It's simple.
1. Create appliance instance of `dudanov::midea::ac::AirConditioner`.
2. Set serial stream interface and communication mode to `9600 8N1`.
3. Add `setup()` and `loop()` methods to the same-named global functions of the project.
4. Control device via `void control(const Control &control)` with optional parameters.
5. You may optionally add your callback function for receive state changes notifications.

```cpp
#include <Arduino.h>
#include <Appliance/AirConditioner/AirConditioner.h>

dudanov::midea::ac::AirConditioner ac;

// Example how can change work mode easily
static inline void switchMode(dudanov::midea::Mode mode) {
  dudanov::midea::ac::Control control;
  control.mode = mode;
  ac.control(control);
}

// Example how can change mode and temp in same time
static inline void changeState(dudanov::midea::Mode mode, float targetTemp) {
  dudanov::midea::ac::Control control;
  control.mode = mode;
  control.targetTemp = targetTemp;
  ac.control(control);
}

// Example how can change mode to AUTO and set target temp to 25C
static inline void changeAuto25() {
  dudanov::midea::ac::Control control;
  control.mode = dudanov::midea::Mode::MODE_AUTO;
  control.targetTemp = 25.0f;
  ac.control(control);
}

// Here you may get new properties states
void onStateChange() {
  ac.getTargetTemp();
  ac.getIndoorTemp();
  ac.getMode();
  ac.getPreset();
  ac.getSwingMode();
  ac.getFanMode();
}

void setup() {
  Serial.begin(9600);     // set serial baudrate to 9600 8N1
  ac.setStream(&Serial);  // set stream serial interface
  ac.addOnStateCallback(onStateChange); // add callback
  ac.setup();
}

void loop() {
  ac.loop();
}
```
