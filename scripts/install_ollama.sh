#!/bin/bash
# install_ollama.sh - Integra Ollama a JuliOS
# Este script prepara la integración del modelo de IA

set -e

echo "========================================="
echo "  JuliOS - Integración de Ollama"
echo "========================================="

# Verificar que Ollama esté instalado en el host
check_ollama() {
    if ! command -v ollama &> /dev/null; then
        echo "Error: Ollama no está instalado en el host"
        echo "Instala con: curl -fsSL https://ollama.com/install.sh | sh"
        exit 1
    fi
    
    echo "Ollama encontrado: $(ollama --version)"
}

# Listar modelos disponibles
list_models() {
    echo ""
    echo "Modelos disponibles:"
    ollama list
    echo ""
}

# Descargar modelo para JuliOS (ligero y offline)
download_model() {
    local model=${1:-minimax-m2.5:cloud}
    
    echo "Descargando modelo: $model"
    echo "Esto puede tomar varios minutos..."
    
    ollama pull "$model"
    
    echo "Modelo descargado exitosamente"
}

# Verificar que el servicio esté corriendo
check_service() {
    if pgrep -x "ollama" > /dev/null; then
        echo "Ollama servicio: CORRIENDO"
    else
        echo "Ollama servicio: NO CORRIENDO"
        echo "Inicia con: ollama serve"
    fi
}

# Crear script de inicio para iaos_light
create_service_script() {
    local output="$1"
    
    cat > "$output" << 'EOF'
#!/bin/bash
# init_iaos.sh - Script de inicio para iaos_light
# Se ejecuta al iniciar el sistema

echo "=== JuliOS IAOS Light ==="
echo "Iniciando proceso de IA autónoma..."

# Verificar que Ollama esté corriendo
if ! pgrep -x "ollama" > /dev/null; then
    echo "Iniciando Ollama..."
    ollama serve &
    sleep 3
fi

# Ejecutar iaos_light en modo daemon
cd /root
./iaos_light -d

echo "iaos_light iniciado"
EOF

    chmod +x "$output"
    echo "Script de inicio creado: $output"
}

# Mostrar ayuda
show_help() {
    echo "Uso: $0 [comando]"
    echo ""
    echo "Comandos:"
    echo "  check       - Verificar instalación de Ollama"
    echo "  list        - Listar modelos disponibles"
    echo "  download    - Descargar modelo (default: minimax-m2.5:cloud)"
    echo "  service     - Crear script de inicio para iaos_light"
    echo "  all         - Ejecutar todos los pasos"
    echo ""
    echo "Ejemplos:"
    echo "  $0 check"
    echo "  $0 download llama3.2:1b"
    echo "  $0 service /etc/init.d/iaos"
}

# Función principal
main() {
    local cmd=${1:-help}
    
    case $cmd in
        check)
            check_ollama
            check_service
            ;;
        list)
            list_models
            ;;
        download)
            local model=${2:-minimax-m2.5:cloud}
            download_model "$model"
            ;;
        service)
            create_service_script "${2:-/root/init_iaos.sh}"
            ;;
        all)
            check_ollama
            check_service
            list_models
            download_model
            create_service_script
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            echo "Comando desconocido: $cmd"
            show_help
            exit 1
            ;;
    esac
}

main "$@"