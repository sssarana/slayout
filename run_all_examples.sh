#!/bin/bash

EXAMPLES_DIR="examples"
COMPILER="./build/bin/slayoutc"

if [ ! -f "$COMPILER" ]; then
  echo "Error: Compiler not found at $COMPILER"
  exit 1
fi

for dir in "$EXAMPLES_DIR"/*/; do
  echo "Processing example: $dir"

  SFILE=$(find "$dir" -name "*.slayout" | head -n 1)
  SHADER=$(find "$dir" -name "*.shader" | head -n 1)
  OUTDIR="${dir}output"

  if [ -z "$SFILE" ] || [ -z "$SHADER" ]; then
    echo "  Skipping: Missing .slayout or .shader file"
    continue
  fi

  mkdir -p "$OUTDIR"

  echo "  Running: $COMPILER $SFILE $SHADER $OUTDIR"
  $COMPILER "$SFILE" "$SHADER" "$OUTDIR"

  if [ $? -ne 0 ]; then
    echo "  Error occurred while processing $dir"
  else
    echo "  Output written to $OUTDIR"
  fi

  echo ""
done
