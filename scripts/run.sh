#!/bin/bash
# Run JuliOS in QEMU

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
JULIOS_DIR="$(dirname "$SCRIPT_DIR")"

KERNEL="$JULIOS_DIR/buildroot-2024.02/output/images/bzImage"
INITRD="$JULIOS_DIR/buildroot-2024.02/output/images/rootfs.cpio.gz"

if [ ! -f "$KERNEL" ] || [ ! -f "$INITRD" ]; then
    echo "Error: Imágenes no encontradas"
    exit 1
fi

echo "🚀 Iniciando JuliOS..."
echo "Saliendo: Ctrl+A X"
echo ""

qemu-system-x86_64 \
    -kernel "$KERNEL" \
    -initrd "$INITRD" \
    -serial mon:stdio \
    -display none \
    -append "console=ttyS0,115200" \
    -m 512
