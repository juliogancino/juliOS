#!/bin/bash
# debug.sh - Script de debugging para JuliOS
# Analiza problemas y muestra información de diagnóstico

echo "========================================="
echo "  JuliOS - Debug y Diagnóstico"
echo "========================================="

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Rutas
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JULIOS_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILDROOT_DIR="$JULIOS_DIR/buildroot-2024.02"

# Función de impresión con color
print_header() {
    echo ""
    echo -e "${BLUE}=== $1 ===${NC}"
}

print_ok() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# 1. Información del sistema
print_header "Información del Sistema"
echo "Fecha: $(date)"
echo "Uptime: $(uptime)"
echo "Kernel: $(uname -r)"
echo "Arquitectura: $(uname -m)"

# 2. Espacio en disco
print_header "Espacio en Disco"
df -h / | tail -1 | awk '{print "Raíz: " $4 " disponibles de " $2 " (" $5 " usado)"}'

# 3. Memoria
print_header "Memoria"
free -h | grep -E "Mem|Swap"

# 4. Procesos relevantes
print_header "Procesos Relevantes"
echo "QEMU:"
ps aux | grep qemu | grep -v grep | awk '{print "  PID " $2 ": " $11 " " $12 " " $13}'
echo "Ollama:"
ps aux | grep ollama | grep -v grep | awk '{print "  PID " $2 ": " $11 " " $12}'
echo "iaos_light:"
ps aux | grep iaos_light | grep -v grep | awk '{print "  PID " $2 ": " $11 " " $12}'

# 5. Estado del build
print_header "Estado del Build"
if [ -f "$BUILDROOT_DIR/output/images/bzImage" ]; then
    print_ok "Kernel compilado: $(ls -lh $BUILDROOT_DIR/output/images/bzImage | awk '{print $5}')"
else
    print_error "Kernel NO compilado"
fi

if [ -f "$BUILDROOT_DIR/output/images/rootfs.cpio" ]; then
    print_ok "RootFS disponible: $(ls -lh $BUILDROOT_DIR/output/images/rootfs.cpio | awk '{print $5}')"
elif [ -f "$BUILDROOT_DIR/output/images/rootfs.cpio.gz" ]; then
    print_ok "RootFS (gz) disponible: $(ls -lh $BUILDROOT_DIR/output/images/rootfs.cpio.gz | awk '{print $5}')"
else
    print_error "RootFS NO disponible"
fi

# 6. Módulo kernel
print_header "Módulo Kernel"
if [ -f "$JULIOS_DIR/src/kernel_patches/iaos_kernel.ko" ]; then
    print_ok "iaos_kernel.ko compilado"
    modinfo "$JULIOS_DIR/src/kernel_patches/iaos_kernel.ko" 2>/dev/null | head -5 || true
else
    print_warn "Módulo kernel no compilado"
fi

# 7. iaos_light
print_header "iaos_light"
if [ -f "$JULIOS_DIR/src/iaos_light/iaos_light" ]; then
    print_ok "iaos_light compilado"
    file "$JULIOS_DIR/src/iaos_light/iaos_light"
else
    print_warn "iaos_light no compilado"
fi

# 8. Ollama
print_header "Ollama"
if command -v ollama &> /dev/null; then
    print_ok "Ollama instalado: $(ollama --version)"
    if pgrep -x "ollama" > /dev/null; then
        print_ok "Ollama corriendo"
    else
        print_warn "Ollama no está corriendo"
    fi
    
    echo "Modelos disponibles:"
    ollama list 2>/dev/null | tail -n +2 || print_warn "No hay modelos"
else
    print_error "Ollama no instalado"
fi

# 9. Dispositivo /dev/iaos
print_header "Dispositivo /dev/iaos"
if [ -e "/dev/iaos" ]; then
    print_ok "Dispositivo existe"
    ls -la /dev/iaos
else
    print_warn "Dispositivo NO existe (¿módulo cargado?)"
fi

# 10. Últimas líneas del log del kernel
print_header "Mensajes del Kernel (dmesg)"
dmesg | tail -10 | grep -E "iaos|IAOS|kernel" || print_warn "Sin mensajes de IAOS"

# 11. /proc/iaos
print_header "/proc/iaos"
if [ -f "/proc/iaos" ]; then
    cat /proc/iaos
else
    print_warn "/proc/iaos no existe"
fi

# 12. Build log
print_header "Build Log (últimas 10 líneas)"
if [ -f "$BUILDROOT_DIR/output/build/build-time.log" ]; then
    tail -10 "$BUILDROOT_DIR/output/build/build-time.log"
else
    print_warn "Log de build no disponible"
fi

echo ""
echo "========================================="
echo "  Debug completado"
echo "========================================="