# Instalación de JuliOS

## Requisitos

- Ubuntu 24.04 o similar
- GCC 13+ 
- QEMU 8.0+
- 20GB espacio en disco
- 4GB RAM mínima

## Pasos de Instalación

### 1. Clonar o extraer proyecto

```bash
cd /ruta/donde/clono/el/repositorio
# El proyecto ya está en julios/
```


### 2. Compilar el sistema

```bash
cd julios
./scripts/build.sh
```

Esto:
- Configura Buildroot
- Compila el kernel Linux 6.1
- Crea rootfs con BusyBox
- Genera imagen bootable

Tiempo estimado: 20-40 minutos

### 3. Ejecutar en QEMU

```bash
./scripts/run.sh
```

O con opciones personalizadas:

```bash
./scripts/run.sh -m 2G -c 2
```

### 4. Dentro de la VM

```bash
# Login: root (sin password)

# Verificar sistema
uname -a
ls -la /

# (si está configurado) iniciar IA
./iaos_light -d
```

## Integración con Ollama

### En el host (no en la VM):

```bash
# Verificar Ollama
ollama list

#确保 modelo disponible
ollama pull minimax-m2.5:cloud
```

### En la VM:

El servicio iaos_light se conecta al host vía red o socket compartido.

## Solución de Problemas

### Error: QEMU no inicia

```bash
# Verificar KVM
ls -la /dev/kvm

# Si no hay KVM, usar software emulation:
./scripts/run.sh --no-kvm
```

### Error: No bootable image

```bash
# Verificar build
ls -la buildroot-2024.02/output/images/
```

### Error: Permisos

```bash
# Agregar usuario a kvm
sudo usermod -aG kvm $USER
```

## Uso Avanzado

### Agregar paquetes extra

```bash
cd buildroot-2024.02
make menuconfig
# Agregar paquetes
make
```

### Custom kernel config

```bash
cd buildroot-2024.02
make linux-menuconfig
make
```

### Debug

```bash
./scripts/debug.sh
```

## Estructura de Directorios

```
julios/
├── buildroot-2024.02/   # Buildroot source
├── src/                  # Código fuente
│   ├── iaos_light/      # Proceso IA
│   └── kernel_patches/   # Módulo kernel
├── scripts/              # Scripts de utilidad
├── config/               # Configuraciones
└── files/                # Archivos para rootfs
```

## Notas

- El build puede tomar 20-40 minutos
- QEMU requiere KVM para mejor rendimiento
- La VM tiene ~256MB RAM por defecto
- Para más debug: `dmesg` en la VM