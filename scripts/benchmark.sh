#!/bin/bash
# benchmark.sh - Benchmarks para JuliOS
# Mide rendimiento del sistema kernel-IA

set -e

echo "========================================="
echo "  JuliOS - Benchmarks"
echo "========================================="

ITERATIONS=${1:-100}
DEVICE="/dev/iaos"

# Verificar dispositivo
if [ ! -e "$DEVICE" ]; then
    echo "Error: $DEVICE no existe"
    exit 1
fi

# Warm-up
echo "Warming up..."
for i in {1..10}; do
    echo "test" > $DEVICE 2>/dev/null || true
done

# Benchmark 1: Latencia de escritura
echo ""
echo "=== Benchmark 1: Latencia de escritura ==="
START=$(date +%s%N)
for i in $(seq 1 $ITERATIONS); do
    echo "test-$i" > $DEVICE 2>/dev/null || true
done
END=$(date +%s%N)

ELAPSED=$((($END - $START) / 1000000))
echo "Tiempo total: ${ELAPSED}ms"
echo "Promedio por operación: $((ELAPSED / ITERATIONS))ms"
echo "Throughput: $((ITERATIONS * 1000 / ELAPSED)) ops/sec"

# Benchmark 2: Diferentes tamaños de mensaje
echo ""
echo "=== Benchmark 2: Diferentes tamaños de mensaje ==="

for size in 10 100 500 1000 2000; do
    MSG=$(printf 'a%.0s' $size)
    
    START=$(date +%s%N)
    for i in $(seq 1 10); do
        echo "$MSG" > $DEVICE 2>/dev/null || true
    done
    END=$(date +%s%N)
    
    ELAPSED=$((($END - $START) / 1000000))
    echo "Mensaje $size bytes: ${ELAPSED}ms (promedio)"
done

# Benchmark 3: Lectura y escritura
echo ""
echo "=== Benchmark 3: Lectura + Escritura (round-trip) ==="
START=$(date +%s%N)
for i in $(seq 1 50); do
    echo "ping" > $DEVICE 2>/dev/null || true
    cat $DEVICE > /dev/null 2>/dev/null || true
done
END=$(date +%s%N)

ELAPSED=$((($END - $START) / 1000000))
echo "Tiempo total: ${ELAPSED}ms"
echo "Promedio: $((ELAPSED / 50))ms por round-trip"

# Benchmark 4: Concurrencia
echo ""
echo "=== Benchmark 4: Concurrencia ==="
echo "Ejecutando 5 procesos simultáneos..."

START=$(date +%s%N)
for j in {1..5}; do
    (
        for i in $(seq 1 20); do
            echo "concurrent-$j-$i" > $DEVICE 2>/dev/null || true
        done
    ) &
done
wait
END=$(date +%s%N)

ELAPSED=$((($END - $START) / 1000000))
echo "Tiempo total: ${ELAPSED}ms"
echo "Throughput total: $((100 * 1000 / ELAPSED)) ops/sec"

echo ""
echo "========================================="
echo "  Benchmarks completados"
echo "========================================="