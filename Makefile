# Makefile principal de JuliOS
# Uso: make <target>

.PHONY: all clean build run test help

# Directorios
JULIOS_DIR := /home/julio/.openclaw/workspace/julios
BUILDROOT_DIR := $(JULIOS_DIR)/buildroot-2024.02
SRC_DIR := $(JULIOS_DIR)/src

# Objetivos por defecto
all: help

# Ayuda
help:
	@echo "JuliOS Makefile"
	@echo "================"
	@echo ""
	@echo "Targets disponibles:"
	@echo "  build       - Compilar el sistema completo"
	@echo "  rebuild     - Limpiar y compilar de nuevo"
	@echo "  run         - Ejecutar en QEMU"
	@echo "  test        - Ejecutar tests"
	@echo "  iaos        - Compilar iaos_light"
	@echo "  kernel      - Compilar módulo kernel"
	@echo "  clean       - Limpiar archivos"
	@echo "  help        - Mostrar esta ayuda"
	@echo ""

# Compilar todo
build:
	@echo "Iniciando build de JuliOS..."
	cd $(BUILDROOT_DIR) && make -j$$(nproc)

# Rebuild
rebuild: clean build

# Ejecutar en QEMU
run:
	$(JULIOS_DIR)/scripts/run.sh

# Tests
test:
	$(JULIOS_DIR)/scripts/test_iaos.sh

# Compilar iaos_light
iaos:
	cd $(SRC_DIR)/iaos_light && $(MAKE)

# Compilar módulo kernel
kernel:
	cd $(SRC_DIR)/kernel_patches && $(MAKE)

# Limpiar
clean:
	$(JULIOS_DIR)/scripts/cleanup.sh

# Debug
debug:
	$(JULIOS_DIR)/scripts/debug.sh

# Status
status:
	@echo "Estado del build:"
	@ls -la $(BUILDROOT_DIR)/output/images/ 2>/dev/null || echo "No hay imágenes"
	@echo ""
	@echo "Últimas actividades:"
	@tail -5 $(BUILDROOT_DIR)/output/build/build-time.log 2>/dev/null || echo "Sin log"