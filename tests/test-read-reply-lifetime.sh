#!/bin/bash
set -euo pipefail
root=$(cd "$(dirname "$0")/.." && pwd)
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
sources=()
for file in "$root"/lib/*.c; do
  [[ "${file##*/}" == libsmb2.c ]] || sources+=("$file")
done
clang -DHAVE_CONFIG_H=1 '-D_U_=__attribute__((unused))' \
  -I"$root/include" -I"$root/include/apple" -I"$root/include/smb2" -I"$root/lib" \
  "$root/tests/read-reply-lifetime.c" "${sources[@]}" -o "$out/read-reply-lifetime"
"$out/read-reply-lifetime"
