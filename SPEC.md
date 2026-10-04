# JuliOS - Especificaciones del Proyecto

## Objetivo
Sistema operativo con IA autónoma integrada en el kernel para pruebas de funcionamiento y artículo científico.

## Stack Tecnológico
- **Kernel:** Linux minimal (Buildroot)
- **IA:** Ollama con modelo ligero (offline)
- **Arquitectura:** x86
- **Virtualización:** QEMU
- **Código:** Español, minimalista, bien comentado

## Arquitectura

```
+------------------+
|    Usuario       |
+--------+---------+
         |
+--------v---------+
|  Userspace      |
|  - iaos_light   | <-- Proceso优先级 máxima
|  - Ollama       |
+--------+---------+
         |
+--------v---------+
|  Syscalls       |
|  - ia_send()    |
|  - ia_recv()    |
+--------+---------+
         |
+--------v---------+
|  Kernel Linux   |
+------------------+
```

## Syscalls Nuevos

### ia_send(mensaje)
- Envía un mensaje desde el kernel/userspace a la IA
- Parámetro: string con el mensaje
- Retorna: código de estado

### ia_recv(buffer)
- Recibe respuesta de la IA
- Parámetro: buffer para almacenar respuesta
- Retorna: tamaño de respuesta o código de error

## Proceso iaos_light
- Primer proceso en user-space (PID 1 o próximo)
- Prioridad máxima (nice -20 o SCHED_FIFO)
- Comunicação con kernel via syscalls ia_send/ia_recv
- Autonomía: ejecuta decisiones propias y responde a comandos

## Modelo de IA
- Ligero y funcional offline
- A decidir: minimax:cloud u otro más pequeño

## Fases de Desarrollo

1. **Setup:** Instalar QEMU + Buildroot
2. **Kernel base:** Compilar Linux minimal con Buildroot
3. **Syscalls:** Agregar ia_send e ia_recv
4. **IA integrada:** Instalar Ollama + modelo
5. **Proceso iaos_light:** Crear servicio que corre al inicio
6. **Pruebas:** Tests de funcionamiento
7. **Artículo:** Redactar paper

## Estado
- [ ] Entorno setupeado
- [ ] Kernel base compilado
- [ ] Syscalls implementados
- [ ] IA integrada
- [ ] Pruebas completas
- [ ] Artículo escrito