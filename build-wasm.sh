#!/bin/bash
set -euo pipefail

BUILD_TYPE=Release

# Reset build directory
rm -rf build
mkdir -p build && cd build

# Configure
emcmake cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

# Build
if [[ "$OSTYPE" == "darwin"* ]]; then
  # macOS
  NUM_PROCESSORS=$(sysctl -n hw.ncpu)
else
  NUM_PROCESSORS=$(nproc)
fi
cmake --build . --config $BUILD_TYPE -- -j $NUM_PROCESSORS

rm viewer2.data
rm viewer2.js
rm viewer2.wasm

cp public/viewer2.data .
cp public/viewer2.js .
cp public/viewer2.wasm .
