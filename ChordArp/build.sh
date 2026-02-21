#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# ChordArp build script
# Usage:
#   ./build.sh              — Debug build
#   ./build.sh Release      — Release build
#   ./build.sh clean        — Remove build dir
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="build"

if [ "$BUILD_TYPE" = "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
    exit 0
fi

echo "=== ChordArp build: $BUILD_TYPE ==="

cmake -B "$BUILD_DIR" \
      -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -G Ninja

cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j"$(nproc)"

echo ""
echo "=== Build complete ==="
echo "VST3:       $BUILD_DIR/ChordArp_artefacts/$BUILD_TYPE/VST3/ChordArp.vst3"
echo "Standalone: $BUILD_DIR/ChordArp_artefacts/$BUILD_TYPE/Standalone/ChordArp"
