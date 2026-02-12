#!/bin/bash
# Compile GLSL shaders to SPIR-V using glslc (from Vulkan SDK)
# Usage: ./compile.sh
# Requires: glslc in PATH (part of Vulkan SDK)

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/compiled"

mkdir -p "${OUTPUT_DIR}"

for shader in "${SCRIPT_DIR}"/*.vert "${SCRIPT_DIR}"/*.frag; do
    [ -f "$shader" ] || continue
    filename=$(basename "$shader")
    echo "Compiling ${filename}..."
    glslc "$shader" -o "${OUTPUT_DIR}/${filename}.spv"
    if [ $? -ne 0 ]; then
        echo "ERROR: Failed to compile ${filename}"
        exit 1
    fi
done

echo "All shaders compiled successfully."
