# MideaUART
Arduino framework library for controlling Midea home appliances using the UART protocol.

Control is possible with a custom dongle. You can make it yourself according to numerous instructions on the Internet, or you can buy a ready-made one in [Tindie Shop](https://www.tindie.com/products/24607/), thereby supporting me and my work.

A far from complete list of supported brands:
1. [Midea](https://www.midea.com/)
2. [Electrolux](https://www.electrolux.ru/)
3. [Qlima](https://www.qlima.com/)
4. [Artel](https://www.artelgroup.com/)
5. [Carrier](https://www.carrier.com/)
6. [Comfee](http://www.comfee-russia.ru/)
7. [Inventor](https://www.inventorairconditioner.com/)
8. [Dimstal/Simando](https://www.simando24.de/)

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

using namespace dudanov::midea::ac;

AirConditioner ac;

// Example how can change work mode easily
static inline void switchMode(Mode mode) {
  Control control;
  control.mode = mode;
  ac.control(control);
}

// Example how can change mode and temp in same time
static inline void changeState(Mode mode, float targetTemp) {
  Control control;
  control.mode = mode;
  control.targetTemp = targetTemp;
  ac.control(control);
}

// Example how can change power state
static inline void setPowerState(bool state) {
  ac.setPowerState(state);
}

// Example how can change mode to AUTO and set target temp to 25C
static inline void changeAuto25() {
  Control control;
  control.mode = Mode::MODE_AUTO;
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

## Purifier / ionizer (experimental)

Adds control for the purifier feature, also called Air Magic or ionizer on some
models. See [#29](https://github.com/dudanov/MideaUART/issues/29).

```cpp
ac.addOnPurifierStatusCallback([](bool on) {
  // Update your switch from the AC's reported state.
});

bool queued = ac.setPurifier(true);
```

`true` means the command was queued, not that the AC accepted it. Wait for the
callback before changing the displayed state. Commands need a valid status no
older than 15 seconds; stale, busy and duplicate requests return `false`.
The library does not turn the purifier on or off at startup or reconnect.

`getPurifier()` reads the last confirmed state. Check `hasFreshStatus()` first:
the initial `false` is not a confirmed OFF reading. `setPurifier()` and
`hasFreshStatus()` accept an optional freshness limit in milliseconds.

The original version was tested on one AC with an ESP8266 UART adapter, including
changes made with the remote. This version passes host tests and firmware builds
but hasn't been flashed for a hardware test. Support on other models isn't confirmed.

Run the tests with Python 3 and a C++17 compiler:

```sh
python3 test/purifier/run_tests.py
python3 test/purifier/run_tests.py --sanitize
```

More detail: [test notes](test/purifier/README.md) and
[API reference](docs/purifier-api.json).

## My thanks

to the following people for their contributions to reverse engineering the UART protocol and source code in the following repositories:

* [Mac Zhou](https://github.com/mac-zhou/midea-msmart)
* [Rene Klootwijk](https://github.com/reneklootwijk/midea-uart)
* [NeoAcheron](https://github.com/NeoAcheron/midea-ac-py)

### Your thanks

If this project was useful to you, you can [buy me](https://paypal.me/dudan0v) a Cup of coffee :)
