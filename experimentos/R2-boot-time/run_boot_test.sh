#!/bin/bash
export LC_ALL=C

BASE="$HOME/.openclaw/workspace/julios"
KERNEL="$BASE/linux-6.1/arch/x86/boot/bzImage"
ROOTFS="$BASE/buildroot-2024.02/output/images/rootfs.cpio.gz"

LOG="boot_test.log"
rm -f "$LOG"

START=$(date +%s%N)

qemu-system-x86_64 \
    -kernel "$KERNEL" \
    -initrd "$ROOTFS" \
    -m 512 \
    -append "console=ttyS0,115200" \
    -nographic \
    > "$LOG" 2>&1 &

QEMU_PID=$!

FOUND=0

for i in $(seq 1 600); do
    if grep -q "buildroot login:" "$LOG" 2>/dev/null; then
        END=$(date +%s%N)

        awk -v s="$START" -v e="$END" \
            'BEGIN {printf "%.3f\n",(e-s)/1000000000}'

        FOUND=1
        break
    fi

    sleep 0.1
done

kill "$QEMU_PID" 2>/dev/null
wait "$QEMU_PID" 2>/dev/null

if [ "$FOUND" -ne 1 ]; then
    echo "ERROR: login prompt no detectado" >&2
    exit 1
fi
