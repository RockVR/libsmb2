#!/bin/bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
sources=()
for file in "$root"/lib/*.c; do
  [[ "${file##*/}" == socket.c ]] || sources+=("$file")
done
clang -DHAVE_CONFIG_H=1 '-D_U_=__attribute__((unused))' \
  -I"$root/include" -I"$root/include/apple" -I"$root/include/smb2" -I"$root/lib" \
  "$root/tests/client-lifecycle.c" "${sources[@]}" -o "$out/client-lifecycle"
"$out/client-lifecycle"
