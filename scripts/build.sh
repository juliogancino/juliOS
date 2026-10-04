#!/bin/bash
# build.sh - Compila todo el proyecto JuliOS
# Usage: ./build.sh

set -e

echo "========================================="
echo "  JuliOS Build Script"
echo "========================================="

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Rutas
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JULIOS_DIR="$(dirname "$SCRIPT_DIR")"
BUILDROOT_DIR="$JULIOS_DIR/buildroot-2024.02"
KERNEL_PATCHES_DIR="$JULIOS_DIR/src/kernel_patches"
IAOS_LIGHT_DIR="$JULIOS_DIR/src/iaos_light"

# Verificar que buildroot esté compilado
check_buildroot() {
    if [ ! -d "$BUILDROOT_DIR/output/images" ]; then
        echo -e "${RED}Buildroot no está compilado. Ejecuta 'make' en buildroot primero.${NC}"
        exit 1
    fi
    
    if [ ! -f "$BUILDROOT_DIR/output/images/bzImage" ]; then
        echo -e "${RED}Kernel no encontrado. Compilando buildroot...${NC}"
        cd "$BUILDROOT_DIR"
        make -j$(nproc)
    fi
}

# Compilar iaos_light
build_iaos_light() {
    echo -e "${YELLOW}Compilando iaos_light...${NC}"
    cd "$IAOS_LIGHT_DIR"
    make clean || true
    make
    echo -e "${GREEN}iaos_light compilado${NC}"
}

# Compilar módulo kernel
build_kernel_module() {
    echo -e "${YELLOW}Compilando módulo kernel...${NC}"
    cd "$KERNEL_PATCHES_DIR"
    make clean || true
    make
    echo -e "${GREEN}Módulo kernel compilado${NC}"
}

# Copiar binarios al rootfs
copy_to_rootfs() {
    echo -e "${YELLOW}Copiando archivos al rootfs...${NC}"
    
    # Aquí iría la lógica para copiar al rootfs de buildroot
    # Por ahora solo сообщаем
    
    echo -e "${GREEN}Archivos copiados (stub)${NC}"
}

# Función principal
main() {
    echo "Iniciando build..."
    
    # Paso 1: Verificar buildroot
    check_buildroot
    
    # Paso 2: Compilar iaos_light
    build_iaos_light
    
    # Paso 3: Compilar módulo kernel
    build_kernel_module
    
    # Paso 4: Copiar al rootfs
    copy_to_rootfs
    
    echo -e "${GREEN}========================================="
    echo "  Build completado!"
    echo "=========================================${NC}"
    echo "Ejecuta ./run.sh para probar en QEMU"
}

# Ejecutar
main "$@"