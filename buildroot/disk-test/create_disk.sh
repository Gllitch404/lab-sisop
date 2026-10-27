#!/bin/bash
# Tutorial 2.4 - Secao 2: Criando uma imagem de disco
# Tamanho: 1 GB (512 * 2097152 = 1073741824 bytes)

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
# Se o script estiver em disk-test/, cria no diretorio buildroot/
if [ "$(basename "$DIR")" = "disk-test" ]; then
    TARGET_DIR="$(dirname "$DIR")"
else
    TARGET_DIR="$DIR"
fi

IMAGE_PATH="$TARGET_DIR/sdb.bin"

if [ -f "$IMAGE_PATH" ] && [ "$1" != "--force" ]; then
    echo "Imagem de disco '$IMAGE_PATH' ja existe. Use --force para recriar."
    exit 0
fi

echo "Criando imagem de disco de 1GB: $IMAGE_PATH..."
dd if=/dev/zero of="$IMAGE_PATH" bs=512 count=2097152
echo "Imagem $IMAGE_PATH criada com sucesso!"
