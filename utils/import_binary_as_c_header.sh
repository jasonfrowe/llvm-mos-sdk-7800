#!/usr/bin/env sh
set -eu

if [ "$#" -ne 3 ]; then
  echo "usage: $0 <input-file> <output-header> <symbol-name>" >&2
  exit 1
fi

input_file="$1"
output_header="$2"
symbol_name="$3"

if [ ! -f "$input_file" ]; then
  echo "error: input file not found: $input_file" >&2
  exit 1
fi

tmp_file="${output_header}.tmp"

xxd -i "$input_file" > "$tmp_file"

orig_symbol="$(basename "$input_file" | tr -c 'A-Za-z0-9_' '_')"

sed \
  -e "s/unsigned char [A-Za-z0-9_][A-Za-z0-9_]*\[\]/static const unsigned char ${symbol_name}[]/" \
  -e "s/unsigned int [A-Za-z0-9_][A-Za-z0-9_]*_len/static const unsigned int ${symbol_name}_len/" \
  "$tmp_file" > "$output_header"

rm -f "$tmp_file"
