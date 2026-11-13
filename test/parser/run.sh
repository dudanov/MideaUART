#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
test_build=$(mktemp -d "${TMPDIR:-/tmp}/mideauart-parser.XXXXXX")
trap 'rm -rf "$test_build"' EXIT
flags=(-std=c++17 -Wall -Wextra -Wno-unused-parameter)
if [[ "${SANITIZE:-0}" == 1 ]]; then
  flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
fi
"${CXX:-c++}" "${flags[@]}" -Itest/parser/stubs -Iinclude \
  test/parser/parser_test.cpp src/Helpers/*.cpp src/Frame/*.cpp \
  src/Appliance/ApplianceBase.cpp -o "$test_build/parser-test"
"$test_build/parser-test" test/parser/fixtures/truncated_status.hex \
  test/parser/fixtures/false_header.hex
