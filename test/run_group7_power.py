"""Build and run the Group 7 regression tests without an attached appliance."""
import argparse
from pathlib import Path
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", nargs="+", default=["c++"],
                        help="Compiler command, e.g. --compiler zig c++")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    build = root / ".pio" / "group7-power"
    build.mkdir(parents=True, exist_ok=True)
    executable = build / ("group7-power.exe" if sys.platform == "win32" else "group7-power")
    subprocess.run([
        *args.compiler, "-std=c++20", "-O0", "-g", "-UNDEBUG",
        "-DARDUINO", "-DARDUINO_ARCH_ESP8266",
        "-Itest/stubs", "-Iinclude", "test/group7_power.cpp",
        *[str(p.relative_to(root)) for p in sorted((root / "src").rglob("*.cpp"))],
        "-o", str(executable),
    ], cwd=root, check=True)
    subprocess.run([str(executable)], cwd=root, check=True)


if __name__ == "__main__":
    main()
