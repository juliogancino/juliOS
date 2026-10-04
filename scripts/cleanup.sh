#!/bin/bash
# cleanup.sh - Limpiar archivos de build

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JULIOS_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$JULIOS_DIR/buildroot-2024.02/output"

echo "Limpiando JuliOS..."

# Limpiar output de buildroot
if [ -d "$BUILD_DIR" ]; then
    echo "Limpiando $BUILD_DIR..."
    rm -rf "$BUILD_DIR"/*
fi

# Limpiar archivos temporales
find "$JULIOS_DIR" -name "*.o" -delete 2>/dev/null || true
find "$JULIOS_DIR" -name "*.ko" -delete 2>/dev/null || true
find "$JULIOS_DIR" -name "*.tmp" -delete 2>/dev/null || true
find "$JULIOS_DIR" -name ".tmp*" -type d -exec rm -rf {} + 2>/dev/null || true

echo "Limpieza completada"