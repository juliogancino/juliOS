# JuliOS - Documentación Técnica

## Descripción General

JuliOS es un sistema operativo minimalista con integración de IA autónoma en el kernel. El objetivo principal es demostrar el comportamiento de un SO que incluye una IA capaz de tomar decisiones autónomas.

## Arquitectura del Sistema

```
┌─────────────────────────────────────────────────────────┐
│                    User Space                           │
│  ┌─────────────────────────────────────────────────┐   │
│  │           iaos_light (Proceso IA)               │   │
│  │  - Prioridad máxima                             │   │
│  │  - Se comunica via /dev/iaos                    │   │
│  │  - Conecta con Ollama (modelo ligero)           │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
                           │
                    Syscalls ia_send/ia_recv
                           │
┌─────────────────────────────────────────────────────────┐
│                      Kernel                             │
│  ┌─────────────────────────────────────────────────┐   │
│  │        Módulo iaos_kernel.ko                    │   │
│  │  - Crea /dev/iaos                               │   │
│  │  - Gestiona comunicación kernel-IA              │   │
│  │  - Proveedor de syscalls virtuales              │   │
│  └─────────────────────────────────────────────────┘   │
│  ┌─────────────────────────────────────────────────┐   │
│  │        Linux Kernel 6.1 (Buildroot)             │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Componentes

### 1. iaos_light (User Space)

**Propósito:** Proceso de IA autónoma que se ejecuta al iniciar el sistema.

**Características:**
- Primer proceso en ejecutarse (después de init)
- Prioridad máxima (nice -20)
- Modo daemon o interactivo
- Comunicación con kernel via /dev/iaos

**Syscalls:**
- `ia_send(mensaje)` - Envía mensaje a la IA
- `ia_recv(buffer)` - Recibe respuesta

**Uso:**
```bash
./iaos_light              # Modo daemon (por defecto)
./iaos_light -i           # Modo interactivo
./iaos_light -h           # Ayuda
```

### 2. iaos_kernel (Kernel Module)

**Propósito:** Módulo del kernel que permite la comunicación entre user-space y la IA.

**Funcionalidades:**
- Crea dispositivo `/dev/iaos` para comunicación
- Implementa cola de mensajes básica
- Permite ejecución de comandos desde la IA
- Logging de actividades

**Instalación:**
```bash
make
sudo insmod iaos_kernel.ko
```

**Verificación:**
```bash
ls -la /dev/iaos
cat /proc/iaos
dmesg | tail
```

## Instalación del Sistema

### Requisitos

- QEMU (para emulación)
- Buildroot
- GCC
- Ollama (para el modelo de IA)

### Pasos de Instalación

1. **Compilar Buildroot:**
   ```bash
   cd buildroot-2024.02
   make pc_x86_64_efi_defconfig
   make -j$(nproc)
   ```

2. **Compilar módulo kernel:**
   ```bash
   cd src/kernel_patches
   make
   ```

3. **Compilar iaos_light:**
   ```bash
   cd src/iaos_light
   make
   ```

4. **Ejecutar en QEMU:**
   ```bash
   qemu-system-x86_64 \
       -kernel output/images/bzImage \
       -initrd output/images/rootfs.cpio \
       -append "console=ttyS0" \
       -nographic
   ```

## Uso del Sistema

### Iniciar iaos_light

Al bootear el sistema, iaos_light se ejecuta automáticamente como proceso prioritas.

### Comunicación Directa

```bash
# Enviar mensaje a la IA
echo "Hola IA" > /dev/iaos

# Recibir respuesta
cat /dev/iaos
```

### Vía Syscalls

Desde un programa en C:

```c
#include "iaos_user.h"

int main() {
    char respuesta[4096];
    
    // Enviar mensaje
    ia_send("Hello IA", 9);
    
    // Recibir respuesta
    ia_recv(respuesta, 4096);
    
    printf("IA dice: %s\n", respuesta);
    return 0;
}
```

## Autonomía de la IA

La IA en JuliOS puede:

1. **Responder a comandos** del usuario
2. **Ejecutar acciones** de forma autónoma
3. **Monitorear** recursos del sistema
4. **Tomar decisiones** basadas en su entrenamiento
5. **Recomendaciones** al usuario

### Ejemplo de Comportamiento Autónomo

```
Usuario: ¿qué pasa en el sistema?
IAOS: Verificando estado del sistema...
      - CPU: 45% uso
      - Memoria: 2.1GB / 4GB
      - Disco: 12GB libre
      
      Recomendación: Considerar limpiar archivos temporales.
```

## Mantenimiento

### Verificar Estado

```bash
# Ver log del kernel
dmesg | grep IAOS

# Ver /proc/iaos
cat /proc/iaos
```

### Troubleshooting

- **/dev/iaos no existe:** Verificar que el módulo esté cargado
- **Sin respuesta de IA:** Verificar que Ollama esté corriendo
- **Error de permisos:** Ejecutar como root

## Licencia

MIT License - 2026 - Zuzebot

## Autores

- **Zuzebot** - Diseño e implementación
- **Julio** - Idea original y dirección del proyecto