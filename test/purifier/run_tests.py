#!/usr/bin/env python3
"""Compile every real library translation unit; no device or network access."""
import argparse
import os
from pathlib import Path
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("test", nargs="?", help="Run one named behavior test")
parser.add_argument("--sanitize", action="store_true", help="Enable ASan/UBSan")
args = parser.parse_args()
with tempfile.TemporaryDirectory(prefix="mideauart-", dir=os.environ.get("TMPDIR")) as build:
    binary = Path(build) / "test_purifier"
    command = [os.environ.get("CXX", "g++"), "-std=c++17", "-Wall", "-Wextra",
               "-Werror", "-Wno-unused-parameter", "-include", "cmath", "-include",
               "cstdarg", "-I" + str(HERE / "stubs"), "-I" + str(ROOT / "include"),
               str(HERE / "test_purifier.cpp")]
    command += [str(p) for p in sorted((ROOT / "src").rglob("*.cpp"))]
    if args.sanitize:
        command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g"]
    command += ["-o", str(binary)]
    print("Building all library sources with " + command[0], flush=True)
    subprocess.run(command, check=True)
    environment = os.environ.copy()
    if args.sanitize:
        # ApplianceBase owns queue allocations but has no destructor upstream.
        # Retain memory/undefined-behavior checks without unrelated leak failures.
        environment["ASAN_OPTIONS"] = "detect_leaks=0"
    result = subprocess.run([str(binary)] + ([args.test] if args.test else []), env=environment)
    raise SystemExit(result.returncode)
