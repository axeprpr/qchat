#!/usr/bin/env bash
# ============================================================================
# QChat - Linux/macOS Build Script
# Requirements: Qt 6.5+, CMake 3.21+, GCC/Clang
# ============================================================================

set -e

echo ""
echo "============================================"
echo "   QChat Build Script"
echo "============================================"
echo ""

BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

# Find Qt
if [ -z "${QT_DIR:-}" ] && [ -n "${QT_ROOT_DIR:-}" ]; then
    QT_DIR="$QT_ROOT_DIR"
fi

if [ -z "${QT_DIR:-}" ]; then
    # Try common locations
    for dir in /opt/Qt/6.*/gcc_64 /usr/lib/qt6 "$HOME"/Qt/6.*/gcc_64 "$HOME"/Qt/6.*/macos; do
        if [ -d "$dir" ]; then
            QT_DIR="$dir"
            break
        fi
    done
fi

if [ -n "${QT_DIR:-}" ]; then
    echo "Using Qt at: $QT_DIR"
    CMAKE_PREFIX="-DCMAKE_PREFIX_PATH=$QT_DIR"
else
    echo "Qt not found in common locations, relying on system Qt..."
    CMAKE_PREFIX=""
fi

# Clone FluentUI if not present
if [ ! -f "third_party/FluentUI/CMakeLists.txt" ]; then
    echo ""
    echo "Downloading FluentUI..."
    git clone --depth 1 https://github.com/zhuzichu520/FluentUI.git third_party/FluentUI || {
        echo "[WARNING] FluentUI download failed. Building without FluentUI."
    }
fi

# Build
echo ""
echo "Building QChat ($BUILD_TYPE)..."
echo ""

mkdir -p "$BUILD_DIR"

cmake -B "$BUILD_DIR" -S . \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY="$BUILD_DIR/bin" \
    $CMAKE_PREFIX

cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" --parallel "$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

echo ""
echo "============================================"
echo "   Build complete!"
echo "   Output: $BUILD_DIR/bin"
echo "============================================"
echo ""
