#!/bin/bash
# stress_test.sh - Test de estrés para el sistema kernel-IA
# Envía múltiples mensajes concurrentes para probar estabilidad

set -e

echo "========================================="
echo "  JuliOS - Test de Estrés"
echo "========================================="

# Configuración
NUM_MENSAJES=${1:-100}
DEVICE="/dev/iaos"
DELAY=${2:-0.1}

echo "Configuración:"
echo "  Mensajes: $NUM_MENSAJES"
echo "  Delay: ${DELAY}s"
echo ""

# Verificar dispositivo
if [ ! -e "$DEVICE" ]; then
    echo "Error: $DEVICE no existe"
    exit 1
fi

# Contadores
SUCCESS=0
FAILURES=0

# Función para enviar mensaje de prueba
test_message() {
    local id=$1
    local msg="Test message #$id from stress test - $(date +%s%N)"
    
    # Enviar
    if echo "$msg" > "$DEVICE" 2>/dev/null; then
        ((SUCCESS++))
    else
        ((FAILURES++))
    fi
    
    # Pequeño delay
    sleep $DELAY
}

echo "Iniciando test de estrés..."
echo ""

# Ejecutar en paralelo si tenemos xargs, si no secuencial
if command -v parallel &> /dev/null; then
    seq 1 $NUM_MENSAJES | parallel -j 4 test_message
else
    for i in $(seq 1 $NUM_MENSAJES); do
        test_message $i
        if [ $((i % 10)) -eq 0 ]; then
            echo "Procesados: $i / $NUM_MENSAJES"
        fi
    done
fi

echo ""
echo "========================================="
echo "  Resultados"
echo "========================================="
echo "Exitosos: $SUCCESS"
echo "Fallidos: $FAILURES"
echo ""

if [ $FAILURES -eq 0 ]; then
    echo "✓ Test completado sin errores"
    exit 0
else
    echo "✗ Test completado con errores"
    exit 1
fi