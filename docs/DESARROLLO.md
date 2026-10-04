# Documentación del Desarrollador - JuliOS

## Visión General

JuliOS es un sistema operativo minimalista que integra un agente de IA autónoma en el kernel. El proyecto tiene como objetivo demostrar la viabilidad de un SO donde la IA puede tomar decisiones autónomas y comunicarse con el usuario.

## Arquitectura del Sistema

### Componentes Principales

```
┌─────────────────────────────────────────────────────────┐
│                    User Space                           │
│  ┌─────────────────────────────────────────────────┐   │
│  │           iaos_light (Proceso IA)               │   │
│  │  - PID: primer proceso o cercano al 1          │   │
│  │  - Prioridad: nice -20 (máxima)                 │   │
│  │  - Comunicación: /dev/iaos                     │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
                           │
                    write()/read()
                           │
┌─────────────────────────────────────────────────────────┐
│                      Kernel                             │
│  ┌─────────────────────────────────────────────────┐   │
│  │        iaos_kernel (Módulo)                     │   │
│  │  - Crea /dev/iaos                              │   │
│  │  - Gestiona cola de mensajes                   │   │
│  │  - Buffer de 4096 bytes                       │   │
│  └───────────────────────────────────────────────────────┤
│  ┌─────────────────────────────────────────────────┐   │
│  │        Linux Kernel 6.1 (Buildroot)            │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Diseño del Módulo Kernel (iaos_kernel)

### Estructuras de Datos

```c
struct iaos_dev {
    dev_t           dev_num;      /* Número de dispositivo */
    struct cdev     cdev;         /* Character device */
    struct class    *class;       /* Clase del dispositivo */
    struct device   *device;      /* Dispositivo */
    char            *rx_buffer;   /* Buffer de recepción */
    char            *tx_buffer;   /* Buffer de transmisión */
    size_t          rx_size;      /* Bytes recibidos */
    size_t          tx_size;      /* Bytes a enviar */
    struct mutex    lock;         /* Exclusion mutua */
    wait_queue_head_t read_queue; /* Cola de espera */
    int             has_data;     /* Flag de datos */
};
```

### Funciones Principales

1. **iaos_init()**: Inicializa el módulo
   - Registra número de dispositivo
   - Crea clase y dispositivo
   - Inicializa buffers y mutex
   - Crea entrada en /proc/iaos

2. **iaos_open()**: Se ejecuta al abrir /dev/iaos

3. **iaos_write()**: Recibe datos de user-space
   - Copia datos al buffer interno
   - Despierta procesos esperando lectura

4. **iaos_read()**: Envía datos a user-space
   - Lee del buffer de respuesta
   - Retorna datos al proceso

## Diseño del Proceso iaos_light

### Características

- **Daemon**: Se ejecuta en background
- **Prioridad**: nice -20 (máxima)
- **Comunicación**: /dev/iaos
- **Modo**: poll/select sobre el dispositivo

### Bucle Principal

```
while (1) {
    mensaje = ia_recv()
    if (mensaje) {
        respuesta = procesar_ia(mensaje)
        ia_send(respuesta)
    }
    usleep(100ms)  // Evitar uso excesivo CPU
}
```

## Integración con Ollama

### Inicialización

1. Verificar si Ollama está corriendo
2. Si no, iniciar con `ollama serve &`
3. Cargar modelo ligero (minimax-m2.5:cloud)

### Comunicación

```
User-Space → /dev/iaos → Kernel → iaos_light → Ollama
                              ↓
Ollama → iaos_light → /dev/iaos → Kernel → User-Space
```

## syscalls ia_send / ia_recv

### ia_send(mensaje)

Envía un mensaje desde user-space al kernel/IA.

**Prototipo**:
```c
ssize_t ia_send(const char *msg, size_t len);
```

**Parámetros**:
- `msg`: Puntero al mensaje
- `len`: Longitud del mensaje

**Retorna**:
- Bytes enviados (>0)
- -1 si error (errno modificado)

### ia_recv(buffer)

Recibe respuesta de la IA.

**Prototipo**:
```c
ssize_t ia_recv(char *buffer, size_t size);
```

**Parámetros**:
- `buffer`: Donde almacenar la respuesta
- `size`: Tamaño máximo del buffer

**Retorna**:
- Bytes recibidos (>0)
- -1 si error

##makefiles

### Módulo Kernel

```makefile
obj-m := iaos_kernel.o
KDIR := /ruta/al/kernel/linux
EXTRA_CFLAGS += -DDEBUG -Wall
```

### User-Space

```makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2
TARGET = iaos_light
```

## Pruebas

### Test de Integración

```bash
./scripts/test_iaos.sh
```

### Test de Estrés

```bash
./scripts/stress_test.sh 1000
```

### Comunicación Manual

```bash
# Enviar mensaje
echo "Hola IA" > /dev/iaos

# Recibir respuesta
cat /dev/iaos
```

## Consideraciones de Seguridad

1. **Permisos**: Solo root puede escribir en /dev/iaos por defecto
2. **Validación**: Verificar tamaños de mensaje
3. **Mutex**: Proteger acceso concurrente
4. **Timeouts**: Evitar bloqueos indefinidos

## Rendimiento

- **Latencia**: ~10-50ms por mensaje (sin IA)
- **Throughput**: ~100 msg/segundo
- **Memoria**: ~4KB para buffers

## Limitaciones Conocidas

1. Solo un proceso puede escribir a la vez
2. Buffer limitado a 4KB
3. Sin soporte para múltiples clientes
4. IA debe estar en el host (no en la VM)

## Trabajo Futuro

- [ ] Implementar syscalls reales (no solo device)
- [ ] Múltiples clientes simultáneos
- [ ] Soporte para Ollama embebido
- [ ] Interfaz de red para comunicación host-VM
- [ ] Logging avanzado

## Autores

- **Zuzebot**: Diseño e implementación
- **Julio**: Idea original y dirección del proyecto

## Licencia

MIT License