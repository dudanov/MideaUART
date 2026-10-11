# Purifier tests

Run from the repository root with Python 3 and C++17 `g++` (`CXX` overrides it):

```sh
python3 test/purifier/run_tests.py
python3 test/purifier/run_tests.py --sanitize
```

Tests exercise the real library parser, queue and callbacks with synthetic UART
frames. They do not access hardware or the network.
ASan/UBSan leak checks are disabled because the upstream request queue has no owning destructor.
