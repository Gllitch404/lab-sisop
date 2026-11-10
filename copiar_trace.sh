#!/bin/bash
# Script utilitário para copiar o arquivo trace.dat do rootfs para o host (Codespaces)
# Powered by Aluno Bernardo & Miguel

set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

ROOTFS="buildroot/output/images/rootfs.ext2"

if [ ! -f "$ROOTFS" ]; then
    echo "Erro: Imagem $ROOTFS não encontrada! Compile o Buildroot primeiro."
    exit 1
fi

# Verifica se o e2tools está instalado
if ! command -v e2cp >/dev/null 2>&1; then
    echo "Instalando e2tools..."
    sudo apt-get update && sudo apt-get install -y e2tools
fi

FILENAME="${1:-trace.dat}"

echo "Copiando /root/$FILENAME de $ROOTFS para o host..."
if e2cp "$ROOTFS:/root/$FILENAME" "$FILENAME" 2>/dev/null; then
    echo "Sucesso! Arquivo $FILENAME copiado para o diretório atual."
    ls -lh "$FILENAME"
else
    echo "Aviso: /root/$FILENAME não encontrado no rootfs. Tentando copiar /$FILENAME..."
    if e2cp "$ROOTFS:/$FILENAME" "$FILENAME" 2>/dev/null; then
        echo "Sucesso! Arquivo $FILENAME copiado para o diretório atual."
        ls -lh "$FILENAME"
    else
        echo "Erro: Não foi possível localizar o arquivo $FILENAME na imagem rootfs."
        echo "Certifique-se de que o teste foi gravado no guest com trace-cmd."
        exit 1
    fi
fi

echo ""
echo "Para abrir o KernelShark no Codespace via navegador:"
echo "  curl -fsSL https://gist.githubusercontent.com/miguelxvr/3c123ef67611652af9d2e45eccb8fc0a/raw/ | bash -s -- start"
echo ""
echo "Ou se você tiver X11/KernelShark instalado localmente:"
echo "  kernelshark trace.dat"
