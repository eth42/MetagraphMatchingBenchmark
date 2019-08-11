#!/bin/bash
CURR_DIR=$(pwd)
SCRIPT_DIR="$CURR_DIR/$(dirname "$0")"
make -C "$SCRIPT_DIR" clean
rm -rf "$SCRIPT_DIR/CMakeCache.txt" "$SCRIPT_DIR/*.cmake" "$SCRIPT_DIR/CMakeFiles/" "$SCRIPT_DIR/Makefile"
find "$SCRIPT_DIR" | grep /CMakeCache.txt | xargs -d"\n" rm
make -C "$SCRIPT_DIR/libs/concorde-97" clean
make -C "$SCRIPT_DIR/libs/blossom-iv" clean
make -C "$SCRIPT_DIR/libs/blossom-v" clean
