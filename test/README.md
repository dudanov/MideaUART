# Status validation tests

Run with Python 3 and a C++20 compiler:

```sh
python test/run_status_validation.py
```

The default compiler is `c++`. To select another compiler, use
`--compiler clang++` or `--compiler zig c++`.

The tests compile the library with small Arduino stubs, a fake serial stream
and a manually advanced clock. No appliance or network connection is used.
They cover malformed status rejection before publication/cache updates,
subsequent valid replies, unchanged timeout/retry behavior, control and display
responses, CRC/checksum variants, and short/extended payload boundaries.

`fixtures/status_validation.h` contains twelve unmodified RX frames recorded
on a Carrier 40MPHAQ09XA3 / SLWF-01 Pro using ESPHome 2026.9.0: ten valid replies
and two malformed patterns. All twelve pass the outer checksum. The malformed
replies produced an incorrect 39.5 C target, or OFF with a negative indoor
temperature, before the next valid reply restored the reported state. These
are software RX captures; they do not establish the source of corruption.
The capturing build included local diagnostics and Group 7 support, with
Group 7 sampling disabled and the upstream status-validation path unchanged.

The shorter checksum-only payload is from
[midea-ac-py #11](https://github.com/mill1000/midea-ac-py/issues/11).
Other boundary lengths are synthetic and do not demonstrate support for more
appliance models. The host tests exercise the parser and request queue, not
physical UART timing, Wi-Fi, or long-term device stability.
