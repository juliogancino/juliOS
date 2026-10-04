# Makefile para iaos_light
# Proceso de IA autónoma en user-space

CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS =

# Nombre del ejecutable
TARGET = iaos_light

# Archivos fuente
SRC = iaos_light.c

# Objetos
OBJS = $(SRC:.c=.o)

# Si hay header local
HEADERS = 

# Por defecto, compilar con syscalls
ifdef DEBUG
    CFLAGS += -DDEBUG -g
endif

.PHONY: all clean install run help

# Compilar
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "=== Compilado: $(TARGET) ==="

# Archivo objeto
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Limpiar
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Limpio"

# Instalar (copiar a ubicación del sistema)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/
	sudo chmod +x /usr/local/bin/$(TARGET)
	@echo "Instalado en /usr/local/bin/"

# Ejecutar
run: $(TARGET)
	sudo ./$(TARGET)

# Help
help:
	@echo "=== Makefile iaos_light ==="
	@echo "make         - Compilar"
	@echo "make clean   - Limpiar"
	@echo "make install - Instalar"
	@echo "make run     - Ejecutar (requiere root)"
	@echo "make help    - Esta ayuda"