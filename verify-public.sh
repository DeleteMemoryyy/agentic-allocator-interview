#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
build_dir=$(mktemp -d "${TMPDIR:-/tmp}/allocator-public.XXXXXX")
trap 'rm -rf "$build_dir"' EXIT INT TERM
cc=${CC:-clang}
"$cc" -std=c11 -Wall -Wextra -Werror -O1 -g -fsanitize=address,undefined \
  -I "$script_dir" "$script_dir/allocator.c" "$script_dir/public_runner.c" \
  -o "$build_dir/runner"
for trace in "$script_dir"/public-tests/*.trace; do
  "$build_dir/runner" "$trace"
done
echo "PUBLIC PASS"
