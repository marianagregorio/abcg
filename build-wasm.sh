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

rm "atividade03.data"
rm "atividade03.js"
rm "atividade03.wasm"

cp "public/atividade03.data" .
cp "public/atividade03.js" .
cp "public/atividade03.wasm" .
