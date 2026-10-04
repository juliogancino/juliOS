# Makefile para el módulo kernel iaos_kernel
# Compila el módulo del kernel para comunicación kernel-IA

# Directorio actual
PWD := $(shell pwd)

# Nombre del módulo
obj-m := iaos_kernel.o

# Repository root
JULIOS_DIR := $(abspath $(PWD)/../..)

# Kernel build directory
# Override from the command line if required:
# make KDIR=/path/to/kernel/build
KDIR ?= $(JULIOS_DIR)/buildroot-2024.02/output/build/linux-custom

# Flags adicionales
EXTRA_CFLAGS += -DDEBUG -Wall -I$(PWD)

# Objetos adicionales (si los hay)
# iaos_kernel-objs := file1.o file2.o

# Predeterminado: compilar módulo
all:
	@echo "=== Compilando iaos_kernel ==="
	@if [ -d "$(KDIR)" ]; then \
		$(MAKE) -C $(KDIR) M=$(PWD) modules; \
	else \
		echo "Kernel headers no encontrados en $(KDIR)"; \
		echo "Ejecuta 'make menuconfig' en buildroot primero"; \
	fi

# Limpiar
clean:
	@echo "=== Limpiando ==="
	@rm -f *.o *.ko *.mod.c *.mod *.order *.symvers .*.cmd
	@rm -rf .tmp_versions

# Instalar módulo
install:
	@echo "=== Instalando módulo ==="
	@sudo insmod iaos_kernel.ko

# Desinstalar módulo
uninstall:
	@echo "=== Desinstalando módulo ==="
	@sudo rmmod iaos_kernel

# Ver información del módulo
info:
	@echo "=== Info del Módulo ==="
	@echo "Módulo: iaos_kernel"
	@echo "Kernel: $(KDIR)"
	@echo "Objetos: $(obj-m)"

.PHONY: all clean install uninstall info