# Changelog - JuliOS

## [1.0.0] - 2026-03-17

### Added
- Sistema operativo base con Buildroot 2024.02
- Kernel Linux 6.1.x compilado
- Rootfs con BusyBox
- iaos_light - Proceso de IA en user-space
- iaos_kernel - Módulo kernel para comunicación
- Integración con Ollama
- Scripts de build, run, setup, install_ollama
- Documentación completa en español
- QEMU scripts de ejecución
- Tests de integración y estrés
- Benchmarks de rendimiento

### Features
- CLI-only (sin GUI)
- Arquitectura x86_64
- Código comentado en español
- Syscalls ia_send/ia_recv (diseño)
- Comunicación vía /dev/iaos
- Prioridad máxima para iaos_light (nice -20)
- Configuración minimalista optimizada

### Files
- `SPEC.md` - Especificaciones del proyecto
- `README.md` - Documentación general
- `INSTALL.md` - Guía de instalación
- `docs/` - Documentación técnica
  - `README.md`
  - `DESARROLLO.md`
  - `ARQUITECTURA.md`
- `src/` - Código fuente
  - `iaos_light/iaos_light.c`
  - `kernel_patches/iaos_kernel.c`
  - `iaos_client.c`
- `include/iaos_user.h`
- `scripts/` - Utilidades
  - `build.sh`
  - `run.sh`
  - `setup.sh`
  - `install_ollama.sh`
  - `test_iaos.sh`
  - `stress_test.sh`
  - `benchmark.sh`
  - `debug.sh`
  - `load_module.sh`
  - `monitor.sh`
  - `cleanup.sh`
- `config/` - Configuraciones
  - `julios_defconfig`
  - `kernel.config`
- `files/` - Archivos para rootfs
  - `rcS`
  - `interfaces`
  - `hosts`
  - `fstab`
  - `iaos.service`

### Known Issues
- Syscalls aún no implementadas a nivel de kernel (solo diseño)
- Ollama debe correr en host (no embebido)
- Solo un proceso puede escribir a /dev/iaos a la vez

### Next Steps (post-artículo)
- [ ] Implementar syscalls reales en el kernel
- [ ] Soporte multi-cliente
- [ ] Ollama embebido
- [ ] Interfaz de red mejorada
- [ ] Logging avanzado