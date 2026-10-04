#!/bin/bash
# load_module.sh - Cargar módulo kernel iaos
# Usage: ./load_module.sh [unload]

set -e

MODULE_NAME="iaos_kernel"
MODULE_PATH="/root/iaos_kernel.ko"
DEVICE_NAME="iaos"
DEVICE_PATH="/dev/$DEVICE_NAME"

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

echo "=== Cargando módulo kernel iaos ==="

# Función para descargar
unload_module() {
    echo "Descargando módulo..."
    
    if lsmod | grep -q "^$MODULE_NAME"; then
        rmmod $MODULE_NAME
        echo -e "${GREEN}✓${NC} Módulo descargado"
    else
        echo "Módulo no estaba cargado"
    fi
    
    if [ -e "$DEVICE_PATH" ]; then
        rm -f $DEVICE_PATH
        echo "Dispositivo eliminado"
    fi
    
    # Limpiar clase
    if [ -d /sys/class/$DEVICE_NAME ]; then
        rmdir /sys/class/$DEVICE_NAME 2>/dev/null || true
    fi
}

# Función para cargar
load_module() {
    echo "Cargando módulo..."
    
    # Verificar si ya está cargado
    if lsmod | grep -q "^$MODULE_NAME"; then
        echo "El módulo ya está cargado"
        return 0
    fi
    
    # Verificar si existe
    if [ ! -f "$MODULE_PATH" ]; then
        echo -e "${RED}Error: Módulo no encontrado en $MODULE_PATH${NC}"
        return 1
    fi
    
    # Cargar módulo
    if insmod $MODULE_PATH; then
        echo -e "${GREEN}✓${NC} Módulo cargado"
    else
        echo -e "${RED}Error al cargar módulo${NC}"
        dmesg | tail -5
        return 1
    fi
    
    # Verificar que se creó el dispositivo
    sleep 1
    
    if [ -e "$DEVICE_PATH" ]; then
        echo -e "${GREEN}✓${NC} Dispositivo $DEVICE_PATH creado"
        chmod 666 $DEVICE_PATH
    else
        echo -e "${RED}Error: Dispositivo no creado${NC}"
        return 1
    fi
}

# Función para verificar
verify_module() {
    echo "Verificando..."
    
    if lsmod | grep -q "^$MODULE_NAME"; then
        echo -e "${GREEN}✓${NC} Módulo cargado: $(lsmod | grep $MODULE_NAME)"
    else
        echo -e "${RED}✗${NC} Módulo no cargado"
        return 1
    fi
    
    if [ -e "$DEVICE_PATH" ]; then
        echo -e "${GREEN}✓${NC} Dispositivo existe: $(ls -la $DEVICE_PATH)"
    else
        echo -e "${RED}✗${NC} Dispositivo no existe"
        return 1
    fi
    
    # Verificar en /proc
    if [ -f /proc/iaos ]; then
        echo -e "${GREEN}✓${NC} Interfaz /proc/iaos disponible"
        cat /proc/iaos
    fi
    
    # Verificar con dmesg
    echo ""
    echo "Mensajes del kernel:"
    dmesg | grep -i iaos | tail -5
}

# Main
case "${1:-load}" in
    load)
        load_module
        verify_module
        ;;
    unload)
        unload_module
        ;;
    reload)
        unload_module
        load_module
        verify_module
        ;;
    verify)
        verify_module
        ;;
    *)
        echo "Uso: $0 {load|unload|reload|verify}"
        exit 1
        ;;
esac

echo ""
echo "=== Listo ==="