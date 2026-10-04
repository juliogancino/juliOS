# JuliOS - Sistema Operativo con IA en el Kernel

## Descripción

JuliOS es un proyecto de investigación que busca implementar un sistema operativo con integración de IA autónoma en el kernel. El objetivo principal es demostrar el comportamiento de un SO que incluye una IA capaz de tomar decisiones de forma autónoma.

## Características Principales

- **Kernel:** Linux 6.1 (Buildroot minimal)
- **IA:** Ollama con modelo ligero offline
- **Arquitectura:** x86_64
- **Código:** Español, minimalista, bien comentado

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