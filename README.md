# juliOS

## Kernel-Mediated Hybrid and Local CPU-Only AI Inference in a Resource-Constrained Minimalist Linux System

**juliOS** is an experimental research platform for studying AI inference integration in a resource-constrained minimalist Linux environment.

The project evaluates two AI execution strategies:

1. **Kernel-mediated hybrid inference:** a juliOS application uses the experimental `ia_send` and `ia_recv` system calls to communicate through the kernel with host-side AI inference.
2. **Local CPU-only inference:** a compact quantized model executes inside the juliOS guest using `llama.cpp`.

### Experimental platform

- Architecture: x86_64
- Linux kernel: 6.1 series
- Buildroot: 2024.02
- Virtualization: QEMU
- Hybrid model: Qwen2.5-Coder 1.5B through Ollama
- Local model: SmolLM2-135M-Instruct Q4_K_M

> juliOS is an experimental research platform, not a production-ready AI operating system.

### Experimental ABI

The hybrid prototype uses:

- `ia_send`: syscall **451**
- `ia_recv`: syscall **452**

These numbers are assignments of the **experimental juliOS ABI** used in this research. They are not proposed as permanent upstream Linux syscall numbers.

## Estructura del Proyecto

```
julios/
├── SPEC.md                    # Especificaciones técnicas
├── docs/
│   └── README.md              # Documentación detallada
├── src/
│   ├── iaos_light/           # Proceso IA en user-space
│   │   ├── iaos_light.c
│   │   └── Makefile
│   └── kernel_patches/        # Módulo kernel
│       ├── iaos_kernel.c
│       └── Makefile
├── scripts/                   # Scripts de automatización
│   ├── build.sh              # Compilar todo
│   ├── run.sh                # Ejecutar en QEMU
│   ├── setup.sh              # Configuración inicial
│   ├── install_ollama.sh     # Integrar Ollama
│   └── test_iaos.sh          # Pruebas
├── config/
│   └── julios_defconfig      # Config de Buildroot
└── buildroot-2024.02/        # Buildroot (descargado)
```

## Inicio Rápido

### Prerrequisitos

```bash
# Instalar dependencias
sudo apt-get install qemu-system-x86 git gcc make

# Descargar proyecto
git clone https://github.com/tu-usuario/julios.git
cd julios
```

### Compilación

```bash
# Ejecutar script de compilación
cd scripts
./build.sh
```

### Ejecución

```bash
# Ejecutar en QEMU
./run.sh

# O con opciones personalizadas
./run.sh -m 1G -c 2 -g
```

## Uso del Sistema

### Comunicación con la IA

```bash
# Enviar mensaje
echo "Hola IA" > /dev/iaos

# Recibir respuesta
cat /dev/iaos
```

### Syscalls

El sistema implementa dos syscalls nuevas:
- `ia_send(mensaje)` - Envía mensaje a la IA
- `ia_recv(buffer)` - Recibe respuesta de la IA

## Desarrollo

### Compilar módulos

```bash
# Compilar iaos_light
cd src/iaos_light
make

# Compilar módulo kernel
cd src/kernel_patches
make
```

### Pruebas

```bash
./scripts/test_iaos.sh
```

## Estado del Proyecto

- [x] Especificaciones definidas
- [ ] Buildroot compilado
- [ ] Kernel funcional
- [ ] Módulo kernel implementado
- [ ] Proceso iaos_light implementado
- [ ] Ollama integrado
- [ ] Pruebas de funcionamiento

## Licencia

MIT License

## Autores

- **Zuzebot** - Diseño e implementación
- **Julio** - Idea original y dirección

## Referencias

- [Buildroot](https://buildroot.org/)
- [Ollama](https://ollama.ai/)
- [QEMU](https://www.qemu.org/)