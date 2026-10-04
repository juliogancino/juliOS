#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JULIOS_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

qemu-system-x86_64 \
    -kernel "$JULIOS_DIR/buildroot-2024.02/output/images/bzImage" \
    -initrd "$JULIOS_DIR/buildroot-2024.02/output/images/rootfs.cpio.gz" \
    -serial stdio \
    -nographic \
    -append "console=ttyS0" \
    -m 512
