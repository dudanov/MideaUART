# Purifier host regression suite

Run from the repository root:

```sh
python3 test/purifier/run_tests.py
python3 test/purifier/run_tests.py --sanitize
python3 test/purifier/run_tests.py remote_only_status_reaches_feedback
```

Requires Python 3 and C++17 `g++` (`CXX` can select another compiler).
Every `src/**/*.cpp` is compiled, not a copied or simplified parser. The only
platform stubs are a controllable clock and deterministic random source.
`cmath` and `cstdarg` are force-included to satisfy existing upstream platform
header dependencies, without unrelated source edits.

The in-memory Stream exercises actual AA framing, CRC8, legacy additive payload
checksums, frame checksum, real request queue, timeout timers, status parser and
callbacks. All body vectors are generated synthetically; there are no device
captures, names, addresses or credentials. Tests do not send device/network
commands. Temporary binaries are removed after each run and are not committed.

The first test was run against the original parser and failed when its existing
notification path missed a purifier-only change. Detection adapters allow that
baseline run to compile; they do not replace the real library on the feature
path. Feature slices were added and executed RED→GREEN before subsequent slices.
The suite additionally covers preservation, short/legacy status replies, invalid
status revocation, dispatch/retry freshness, superseded snapshots, startup,
remote-only polling, busy controls, duplicate requests, opposite acknowledgements,
timeout and millisecond rollover.

ASan and UBSan are enabled by `--sanitize`. Leak detection is disabled because
upstream ApplianceBase has no queue-owning destructor; this feature does not
change that lifecycle. There is no embedded firmware link/upload test here.

The feature-local gate deliberately does not take over global climate validation
from proposed PR #50. Complete malformed C0 replies can retain upstream climate
parser behavior while being excluded from purifier feedback/write snapshots.
