#!/bin/bash
# monitor.sh - Monitorear build en tiempo real

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JULIOS_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_LOG="$JULIOS_DIR/buildroot-2024.02/output/build/build-time.log"
IMAGES_DIR="$JULIOS_DIR/buildroot-2024.02/output/images"

echo "========================================="
echo "  JuliOS - Monitoreo de Build"
echo "========================================="
echo "Presiona Ctrl+C para salir"
echo ""

# Mostrar última línea cada 2 segundos
tail -f "$BUILD_LOG" 2>/dev/null | while read line; do
    # Limpiar línea anterior
    echo -ne "\r\033[K"
    
    # Extraer nombre del paquete y timestamp
    pkg=$(echo "$line" | grep -oP '(?<=: )(start|end)[^:]+(?= :)' | tail -1)
    echo -n "Último: $pkg"
    
    # Verificar si hay imágenes
    if [ -f "$IMAGES_DIR/bzImage" ] || [ -f "$IMAGES_DIR/rootfs.cpio" ]; then
        echo ""
        echo "========================================="
        echo "  ¡Build completado!"
        echo "========================================="
        ls -la "$IMAGES_DIR"/
        exit 0
    fi
done