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

The generic `AirConditioner` purifier API addresses
[Fresh/ionizer function missing (#29)](https://github.com/dudanov/MideaUART/issues/29).
The candidate mapping is C0 status and 0x40 command body byte 9, bit 5 (`0x20`),
counting the body identifier as byte 0 (full AA-frame byte 19).
The protocol mapping was verified with a local prototype on **one
Midea-compatible AC using an ESP8266 UART adapter**, including ON/OFF control
and physical-remote feedback. This upstream implementation has host-test and
compile-only coverage; it has not been flashed for a separate hardware test.
This is not a claim of universal model support or a capability-detection mechanism.

```cpp
// Register before setup(); publish only confirmed appliance feedback.
ac.addOnPurifierStatusCallback([](bool confirmed) {
  // Update your application's purifier indicator here.
});

// Explicit user action only. true means queued, NOT confirmed by the appliance.
bool queued = ac.setPurifier(true);  // optional maxAge in ms, default 15000
bool fresh = ac.hasFreshStatus();
bool confirmed = ac.getPurifier();
```

`getPurifier()` starts false, but that is not a confirmed OFF value until a
trustworthy status arrives. Freshness is inclusive (`age <= maxAge`); zero
always means not fresh. A write returns false for missing/invalid/stale status,
a busy climate/purifier control request, or an already-confirmed matching value.
Do not publish requested values optimistically or restore a saved value during
startup/reconnection. There are no automatic purifier writes. Callbacks fire
on every trustworthy C0 response, even when unchanged or opposite to a requested
value, so remote-only changes are visible through the normal status polling path.
Existing climate callbacks are not replaced, and purifier-only changes do not
add climate-change notifications. Callback registration does not publish a value.

Writes use a separate trusted snapshot and preserve power, raw mode, raw fan,
swing, target and preset. Target and preset use the existing command encoders
(including ECO status `0x10` to command `0x80`); normal beeper masks, CRC and
frame checksum are retained. Before sending or retrying, a feature-local guard
cancels a write if its snapshot expired, was invalidated, or was superseded by
another status. Cancellation/timeout never publishes the requested state.

### Validation scope and related work

The purifier gate accepts a 16-byte C0 status prefix plus an integrity byte:
CRC8 **or** legacy additive checksum (sum of the body including checksum is zero
modulo 256). The integrity byte is excluded from optional-field decoding in the
purifier snapshot. Shorter complete replies can therefore provide feedback and
seed a write without a blanket 23-byte/CRC8-only restriction. Missing optional
fields use the existing getters' zero defaults; their preservation on other
models is not hardware-verified. Malformed C0 replies revoke purifier freshness
and do not publish purifier feedback, even after a previously valid status.

Global climate checksum validation, integrity-byte removal and malformed-reply
retry policy belong to the separate proposal
[#50](https://github.com/dudanov/MideaUART/pull/50), not this feature. The
feature-local gate follows its described formats but does not import that PR or
assume it merged. Existing climate parsing still accepts complete malformed
C0 replies; this change only adds a minimal bound needed to avoid the existing
`copyStatus()` memcpy reading truncated data. If #50 lands, the validation and
normalization should be consolidated while retaining purifier invalidation.

This feature also leaves existing busy `control()` behavior unchanged; pending
climate-command replay is the separate proposal
[#48](https://github.com/dudanov/MideaUART/pull/48). It is not a dependency of
this implementation. No current upstream ESPHome API compatibility is claimed.
A machine-readable follow-up contract is in [docs/purifier-api.json](docs/purifier-api.json).

### Host regression tests

With Python 3 and a C++17-capable `g++` (or `CXX`):

```sh
python3 test/purifier/run_tests.py
python3 test/purifier/run_tests.py --sanitize
```

These compile every real library translation unit and exercise the frame
receiver, checksums, queue, timers, parser and callbacks using an in-memory stream
and synthetic protocol vectors. They do not contact a device or network. See
[test/purifier/README.md](test/purifier/README.md) for details.

## My thanks

to the following people for their contributions to reverse engineering the UART protocol and source code in the following repositories:

* [Mac Zhou](https://github.com/mac-zhou/midea-msmart)
* [Rene Klootwijk](https://github.com/reneklootwijk/midea-uart)
* [NeoAcheron](https://github.com/NeoAcheron/midea-ac-py)

### Your thanks

If this project was useful to you, you can [buy me](https://paypal.me/dudan0v) a Cup of coffee :)
