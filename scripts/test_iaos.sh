#!/bin/bash
# test_iaos.sh - Pruebas de integración para JuliOS
# Ejecuta pruebas del sistema kernel-IA

set -e

echo "========================================="
echo "  JuliOS - Pruebas de Integración"
echo "========================================="

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Rutas
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JULIOS_DIR="$(dirname "$SCRIPT_DIR")"

# Tests passing/failing counters
TESTS_PASSED=0
TESTS_FAILED=0

# Función para reportar resultado
test_pass() {
    echo -e "${GREEN}✓ PASS:${NC} $1"
    ((TESTS_PASSED++))
}

test_fail() {
    echo -e "${RED}✗ FAIL:${NC} $1"
    ((TESTS_FAILED++))
}

test_info() {
    echo -e "${YELLOW}→ INFO:${NC} $1"
}

# Test 1: Verificar que buildroot esté compilado
test_buildroot() {
    echo ""
    echo "=== Test: Buildroot compilado ==="
    
    if [ -f "$JULIOS_DIR/buildroot-2024.02/output/images/bzImage" ]; then
        test_pass "Kernel compilado"
    else
        test_fail "Kernel no encontrado"
        return 1
    fi
    
    if [ -f "$JULIOS_DIR/buildroot-2024.02/output/images/rootfs.cpio" ]; then
        test_pass "RootFS disponible"
    else
        test_fail "RootFS no encontrado"
    fi
}

# Test 2: Verificar iaos_light compilado
test_iaos_light() {
    echo ""
    echo "=== Test: iaos_light ==="
    
    if [ -f "$JULIOS_DIR/src/iaos_light/iaos_light" ]; then
        test_pass "iaos_light compilado"
    else
        test_info "Compilando iaos_light..."
        cd "$JULIOS_DIR/src/iaos_light"
        make
        if [ -f "iaos_light" ]; then
            test_pass "iaos_light compilado"
        else
            test_fail "Error al compilar iaos_light"
        fi
    fi
}

# Test 3: Verificar módulo kernel
test_kernel_module() {
    echo ""
    echo "=== Test: Módulo Kernel ==="
    
    if [ -f "$JULIOS_DIR/src/kernel_patches/iaos_kernel.ko" ]; then
        test_pass "Módulo kernel compilado"
    else
        test_info "Módulo kernel no compilado (opcional)"
    fi
}

# Test 4: Verificar Ollama
test_ollama() {
    echo ""
    echo "=== Test: Ollama ==="
    
    if command -v ollama &> /dev/null; then
        test_pass "Ollama instalado"
        
        if pgrep -x "ollama" > /dev/null; then
            test_pass "Ollama corriendo"
        else
            test_info "Iniciando Ollama..."
            ollama serve &
            sleep 3
            if pgrep -x "ollama" > /dev/null; then
                test_pass "Ollama iniciado"
            else
                test_fail "No se pudo iniciar Ollama"
            fi
        fi
    else
        test_fail "Ollama no instalado"
    fi
}

# Test 5: Verificar QEMU
test_qemu() {
    echo ""
    echo "=== Test: QEMU ==="
    
    if command -v qemu-system-x86_64 &> /dev/null; then
        test_pass "QEMU instalado"
    else
        test_fail "QEMU no instalado"
    fi
}

# Test 6: Crear imagen de prueba
test_image() {
    echo ""
    echo "=== Test: Imagen de prueba ==="
    
    local test_img="$JULIOS_DIR/test.img"
    
    # Crear imagen de 100MB
    dd if=/dev/zero of="$test_img" bs=1M count=100 2>/dev/null
    
    if [ -f "$test_img" ]; then
        test_pass "Imagen de prueba creada"
    else
        test_fail "No se pudo crear imagen"
    fi
}

# Test 7: Ejecutar en modo dry-run QEMU
test_qemu_dryrun() {
    echo ""
    echo "=== Test: QEMU dry-run ==="
    
    cd "$JULIOS_DIR"
    
    if [ -f "buildroot-2024.02/output/images/bzImage" ]; then
        # Solo verificar que el comando QEMU no dé error de sintaxis
        if qemu-system-x86_64 -help > /dev/null 2>&1; then
            test_pass "QEMU funcional"
        else
            test_fail "QEMU no funciona"
        fi
    else
        test_info "Buildroot no compilado aún"
    fi
}

# Resumen de resultados
show_summary() {
    echo ""
    echo "========================================="
    echo "  Resumen de Pruebas"
    echo "========================================="
    echo -e "Pasadas: ${GREEN}$TESTS_PASSED${NC}"
    echo -e "Fallidas: ${RED}$TESTS_FAILED${NC}"
    echo ""
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}✓ Todas las pruebas pasaron${NC}"
        return 0
    else
        echo -e "${RED}✗ Algunas pruebas fallaron${NC}"
        return 1
    fi
}

# Función principal
main() {
    # Ejecutar todos los tests
    test_qemu
    test_buildroot
    test_iaos_light
    test_kernel_module
    test_ollama
    test_image
    test_qemu_dryrun
    
    # Mostrar resumen
    show_summary
}

main "$@"