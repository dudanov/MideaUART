# Frame receiver regression tests

Run from the repository root with a C++17 compiler:

```sh
bash test/parser/run.sh
SANITIZE=1 bash test/parser/run.sh
```

The host stubs supply only the ESP-IDF clock/random functions. Tests exercise
the production `ApplianceBase`, `Frame`, timer and request-dispatch code through
a fake `Stream`; they do not copy the parser implementation. Request handlers
in this suite explicitly check the expected body and CRC. Appliance-specific
handlers are outside this change.

The anonymized receiver fixtures reproduce [issue #57](https://github.com/dudanov/MideaUART/issues/57):

- `truncated_status.hex`: 80 received bytes, consisting of a 39-byte truncated
  candidate and an intact 41-byte status frame at offset 39. The original reader
  consumes the intact frame's `AA 28` into the failed candidate, then loses it.
- `false_header.hex`: 96 received bytes, with identical intact 33-byte replies
  at offsets 30 and 63. A false start at offset 29 produces `AA AA 20`, advertising
  171 bytes with the wrong appliance type. Suffix-only recovery without header
  validation can stall here.

These are software-received bytes, not wire timestamps. Timed request tests model
delivery; they do not establish the cause of original byte corruption. Neither
fixture contains room labels, network addresses, credentials or device serials.

Coverage includes every two-part split of each fixture, byte-by-byte delivery,
configured non-AC appliance types, nested and consecutive valid frames, invalid
lengths/checksums, 499/500 ms expiry, queued continuation after a delayed loop,
unreadable available bytes, clock wrap, the 256-byte candidate boundary, request
success/retry/exhaustion, CRC-checking dispatch and one million seeded noise bytes.

The 500 ms expiry is software-observed inactivity and is not a proven protocol
inter-byte limit. A valid continuation arriving after expiry can be discarded.
Complete outer-checksum-valid frames remain indivisible. Appliance-type filtering
should be reviewed for broadcast/discovery use before adopting this for other
appliances. The parser adds a 256-byte pending-input buffer plus bookkeeping.
