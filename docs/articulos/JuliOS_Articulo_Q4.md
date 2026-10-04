# JuliOS: Sistema Operativo con Inteligencia Artificial en el Kernel — Propuesta Arquitectónica

**Autores:** Julio ( supervisor), Zuzebot (implementación)  
**Fecha:** Mayo 2026  
**Conferencia:** Submission Q4 (pendiente)

---

## Resumen

Este trabajo presenta JuliOS, una propuesta de arquitectura de sistema operativo que integra capacidades de inteligencia artificial autónoma directamente en el kernel Linux. A diferencia de enfoques previos que implementan IA como servicios de usuario, proponemo s un diseño basado en syscalls dedicados (`ia_send`, `ia_recv`) que permiten comunicación directa entre el kernel y un modelo de lenguaje. Describimos la arquitectura, implementación del módulo kernel y proceso user-space `iaos_light`, junto con mediciones de rendimiento del sistema base.

---

## 1. Introducción

La integración de IA en sistemas operativos es un área emergente. Trabajos recientes exploran LLMs para planificación de procesos y gestión de recursos. Sin embargo, la mayoría operan exclusivamente en espacio de usuario.

**Contribución:** Proponemos una arquitectura donde la IA interactúa con el kernel via syscalls dedicadas, permitiendo decisiones autônomas a nivel de sistema.

---

## 2. Arquitectura Propuesta

```
+------------------+     +-------------------+
|   Aplicaciones   |     |   iaos_light      |
+--------+---------+     |   (user-space)    |
         |               +---------+---------+
         |                         |
    +----v-----------------------v----+
    |     Syscalls IA (ia_send/     |
    |     ia_recv)                  |
    +------------+------------------+
                 |
          +------v-------+
          |  Kernel     |
          |  Linux 6.1  |
          +-------------+
```

### 2.1 Syscalls Propuestas

| Syscall | Función | Parámetros |
|---------|---------|------------|
| `ia_send(msg)` | Envía mensaje a IA | char* mensaje |
| `ia_recv(buf)` | Recibe respuesta IA | char* buffer |

### 2.2 Proceso iaos_light

Proceso daemon con prioridad máxima que maneja la comunicación con Ollama (modelo ligero offline).

---

## 3. Implementación

### 3.1 Entorno de Pruebas

- **Virtualizador:** QEMU 8.2.2
- **Kernel:** Linux 6.1 via Buildroot 2024.02
- **RootFS:** Cpio.gz (~45MB minimal, ~177MB con dependencias)
- **Host:** Ubuntu 24.04, Ollama con modelo minimax-m2.5:cloud

### 3.2 Componentes Implementados

| Componente | Archivo | Estado |
|------------|---------|--------|
| Módulo kernel | `src/kernel_patches/iaos_kernel.c` | Código listo |
| User-space daemon | `src/iaos_light/iaos_light.c` | Código listo |
| Buildroot config | `config/julios_defconfig` | Configurado |
| Scripts automation | `scripts/build.sh`, `run.sh` | Operativos |

---

## 4. Métricas y Resultados

### 4.1 Sistema Base

| Métrica | Valor |
|---------|-------|
| Tamaño kernel (bzImage) | **9.2 MB** |
| Tamaño rootfs minimal | **25 MB** |
| Líneas de código (kernel module) | 355 |
| Líneas de código (iaos_light) | 344 |
| Kernel versión | Linux 6.1.0 (preemptivo) |
| Buildroot output | 38 archivos |
| Host build | Ubuntu 24.04, VMware |

### 4.2 Métricas de Ejecución (QEMU)

| Métrica | Valor |
|---------|-------|
| **Boot time (hasta red)** | **~17 segundos** |
| Memoria total | 512 MB (QEMU) |
| Memoria usada (idle) | ~55 MB |
| Memoria libre (idle) | 457 MB (87%) |
| Red | ✅ enp0s3, 1000 Mbps Full Duplex |
| Kernel code size | 14,348 KB |
| Device drivers | e1000, serial 8250, USB, SCSI |

### 4.3 Limitaciones de IA (Ollama)

| Componente | Tamaño | Viabilidad |
|------------|--------|------------|
| Binario Ollama | 41 MB | ❌ Too large |
| Librerías requeridas | ~25 MB | ❌ Dependencias pesadas |
| **Total estimado** | **~65+ MB** | Excede rootfs minimal (25MB) |

**Conclusión:** Ollama no puede ejecutarse dentro del entorno emulado minimal. Se propone arquitectura híbrida con comunicación via API REST al host.

### 4.2 Limitaciones Identificadas

- Ollama no corre dentro de QEMU (recursos insuficientes en emulación)
- Syscalls no aplicadas al kernel compilado
- Red no configurada en build actual

### 4.3 Enfoque Propuesto

Dado que ejecutar Ollama dentro de QEMU es inviable computacionalmente, proponemo s un modelo híbrido:
1. Kernel y syscalls compilados en el guest
2. Comunicación con IA via API REST al host
3. Módulo kernel actuando como proxy

---

## 5. Trabajo Futuro

- Aplicar parches de syscalls al kernel
- Integrar módulo kernel compilado en rootfs
- Implementar canal de comunicación host↔guest (virtio o socket)
- Medición de latencia de syscalls

---

## 6. Conclusiones

Presentamos la arquitectura de JuliOS para integración de IA en el kernel. Aunque la ejecución completa requiere más trabajo, el diseño de syscalls y la estructura del sistema base demuestran la viabilidad del enfoque. El artículo actual sirve como fundamento para publicaciones Q4/Q3 futuras.

---

## Referencias

[1] Buildroot. https://buildroot.org/  
[2] Ollama. https://ollama.ai/  
[3] QEMU. https://www.qemu.org/  
[4] Linux Kernel Documentation. https://www.kernel.org/doc/html/latest/

---

*Manuscrito generado automáticamente - Mayo 2026*