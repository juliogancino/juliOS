#!/bin/bash
# setup.sh - Configuración inicial de JuliOS
# Este script debe ejecutarse como root

set -e

echo "========================================="
echo "  JuliOS - Setup Inicial"
echo "========================================="

# Verificar root
if [ "$EUID" -ne 0 ]; then
    echo "Error: Este script debe ejecutarse como root"
    echo "Ejecuta: sudo $0"
    exit 1
fi

# Rutas
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JULIOS_DIR="$(dirname "$SCRIPT_DIR")"

echo "Instalando dependencias del sistema..."

# Instalar dependencias (si no están)
if ! command -v qemu-system-x86_64 &> /dev/null; then
    echo "Instalando QEMU..."
    apt-get update
    apt-get install -y qemu-system-x86
fi

if ! command -v git &> /dev/null; then
    echo "Instalando Git..."
    apt-get install -y git
fi

if ! command -v gcc &> /dev/null; then
    echo "Instalando GCC..."
    apt-get install -y gcc make
fi

# Descargar y preparar Buildroot
setup_buildroot() {
    if [ -d "$JULIOS_DIR/buildroot-2024.02" ]; then
        echo "Buildroot ya descargado"
    else
        echo "Descargando Buildroot..."
        cd "$JULIOS_DIR"
        wget -q https://buildroot.org/downloads/buildroot-2024.02.tar.gz
        tar xzf buildroot-2024.02.tar.gz
        rm buildroot-2024.02.tar.gz
        echo "Buildroot listo"
    fi
}

# Configurar permisos
setup_permissions() {
    echo "Configurando permisos..."
    
    # Grupo de usuarios para qemu
    if ! getent group kvm > /dev/null; then
        groupadd kvm
    fi
    
    # Permisos para KVM (si está disponible)
    if [ -e /dev/kvm ]; then
        chmod 666 /dev/kvm 2>/dev/null || true
    fi
}

# Verificar espacio
check_space() {
    local avail=$(df -BG / | tail -1 | awk '{print $4}' | tr -d 'G')
    echo "Espacio disponible: ${avail}GB"
    
    if [ "$avail" -lt 10 ]; then
        echo "Advertencia: Se recomienda al menos 10GB libres"
    fi
}

# Función principal
main() {
    check_space
    setup_permissions
    
    echo ""
    echo "Setup completado!"
    echo ""
    echo "Siguientes pasos:"
    echo "  1. cd $JULIOS_DIR/buildroot-2024.02"
    echo "  2. make menuconfig  # Configurar"
    echo "  3. make              # Compilar"
    echo ""
    echo "O usa los scripts de julios/scripts/"
}

main "$@"