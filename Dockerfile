# Dockerfile para compilar JuliOS
# Uso: docker build -t julios-builder .

FROM ubuntu:24.04

# Instalar dependencias
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    wget \
    curl \
    libncurses-dev \
    libssl-dev \
    bc \
    bison \
    flex \
    libelf-dev \
    python3 \
    qemu-system-x86 \
    && rm -rf /var/lib/apt/lists/*

# Directorio de trabajo
WORKDIR /julios

# Copiar archivos del proyecto
COPY . .

# Construir JuliOS
RUN chmod +x scripts/*.sh && \
    ./scripts/setup.sh && \
    cd buildroot-2024.02 && \
    make julios_defconfig && \
    make -j$(nproc)

# Exportar imágenes
FROM scratch
COPY --from=0 /julios/buildroot-2024.02/output/images/bzImage /bzImage
COPY --from=0 /julios/buildroot-2024.02/output/images/rootfs.cpio.gz /rootfs.cpio.gz
COPY --from=0 /julios/scripts/run.sh /run.sh

CMD ["/run.sh"]