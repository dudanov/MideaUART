# Purifier tests

From the repository root:

```sh
python3 test/purifier/run_tests.py
python3 test/purifier/run_tests.py --sanitize
python3 test/purifier/run_tests.py remote_only_status_reaches_feedback
```

Requires Python 3 and a C++17 compiler (`g++` by default; override with `CXX`).
Tests compile all real library sources and exercise the parser, framing, queue,
timers and callbacks with an in-memory Stream and synthetic status bodies.
No hardware or network access; this is not an embedded firmware test.

`--sanitize` enables ASan/UBSan. Leak detection is disabled because the upstream
request queue has no owning destructor.
