#!/bin/bash
# Run JuliOS with GUI window

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
JULIOS_DIR="$(dirname "$SCRIPT_DIR")"

KERNEL="$JULIOS_DIR/buildroot-2024.02/output/images/bzImage"
INITRD="$JULIOS_DIR/buildroot-2024.02/output/images/rootfs.cpio.gz"

echo "🚀 Iniciando JuliOS con GUI..."

qemu-system-x86_64 \
    -kernel "$KERNEL" \
    -initrd "$INITRD" \
    -m 512
