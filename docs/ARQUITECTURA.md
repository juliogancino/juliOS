# Arquitectura JuliOS - Diagrama

```
┌─────────────────────────────────────────────────────────────────────────┐
│                              QEMU/KVM                                    │
│  ┌───────────────────────────────────────────────────────────────────┐  │
│  │                        JuliOS VM                                  │  │
│  │  ┌─────────────────────────────────────────────────────────────┐ │  │
│  │  │                    Linux Kernel 6.1                         │ │  │
│  │  │                                                             │ │  │
│  │  │  ┌─────────────────────────────────────────────────────┐   │ │  │
│  │  │  │              iaos_kernel (Módulo)                     │   │ │  │
│  │  │  │                                                        │   │ │  │
│  │  │  │  /dev/iaos          /proc/iaos                        │   │ │  │
│  │  │  │      ↑                      ↑                        │   │ │  │
│  │  │  └──────┼──────────────────────┼────────────────────────┘   │ │  │
│  │  │         │                      │                            │ │  │
│  │  │  ┌──────┴──────────────────────┴────────────────────────┐   │ │  │
│  │  │  │              iaos_light (Proceso)                      │   │ │  │
│  │  │  │                                                          │   │ │  │
│  │  │  │  - nice -20 (máxima prioridad)                          │   │ │  │
│  │  │  │  - Primera posición en PID                              │   │ │  │
│  │  │  │  - Conexión Ollama vía red/stdout                      │   │ │  │
│  │  │  └─────────────────────────────────────────────────────────┘   │ │  │
│  │  │                                                             │ │  │
│  │  │  ┌─────────────────────────────────────────────────────┐   │ │  │
│  │  │  │              BusyBox (Sistema)                        │   │ │  │
│  │  │  │  sh, ls, cat, mkdir, mount, ifconfig, etc.          │   │ │  │
│  │  │  └─────────────────────────────────────────────────────┘   │ │  │
│  │  └─────────────────────────────────────────────────────────────┘ │  │
│  └───────────────────────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────────────────────┘
                                    │
                        ┌───────────┴───────────┐
                        │      Host Network     │
                        │   192.168.122.1/24   │
                        └───────────┬───────────┘
                                    │
                        ┌───────────┴───────────┐
                        │    Ollama Service     │
                        │   (Host:11434)       │
                        │                      │
                        │  ┌────────────────┐  │
                        │  │minimax-m2.5    │  │
                        │  │:cloud Model    │  │
                        │  └────────────────┘  │
                        └───────────────────────┘


Flujo de datos:
═══════════════

Usuario: "Hola IA" → /dev/iaos → iaos_kernel → iaos_light → Ollama
                     ← /dev/iaos ← iaos_kernel ← iaos_light ← Respuesta


Estructura de archivos:
══════════════════════

/julios/
├── buildroot-2024.02/
│   └── output/
│       └── images/
│           ├── bzImage         (Kernel)
│           └── rootfs.cpio.gz  (Root filesystem)
├── src/
│   ├── iaos_light/
│   │   ├── iaos_light.c       (Código fuente)
│   │   └── Makefile
│   └── kernel_patches/
│       ├── iaos_kernel.c      (Código fuente)
│       └── Makefile
├── scripts/
│   ├── build.sh
│   ├── run.sh
│   └── ...
└── files/
    ├── rcS                    (Init script)
    └── ...


 Syscalls personalizadas (diseño):
 ═══════════════════════════════

┌─────────────────────────────────────────────────────────┐
│                    User Space                           │
│                                                         │
│  iaos_send(msg, len)    →    Syscall 378              │
│  iaos_recv(buf, size)   →    Syscall 379              │
└─────────────────────────────────────────────────────────┘
                              ↓
┌─────────────────────────────────────────────────────────┐
│                    Kernel Space                         │
│                                                         │
│  sys_iaos_send()   →   Copia mensaje al buffer kernel │
│  sys_iaos_recv()   →   Copia respuesta al usuario    │
│                                                         │
│  (Alternativa: usar /dev/iaos con read/write)         │
└─────────────────────────────────────────────────────────┘