#!/bin/bash
# Script utilitário para execução do QEMU com o disco secundário (Tutorial 2.4)
# Comando oficial do Tutorial 2.4:
# qemu-system-i386 --kernel output/images/bzImage --hda output/images/rootfs.ext2 --hdb sdb.bin --nographic --append "console=ttyS0 root=/dev/sda"

set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

BUILDROOT_DIR="$DIR/buildroot"
IMAGES_DIR="$BUILDROOT_DIR/output/images"
KERNEL="$IMAGES_DIR/bzImage"
ROOTFS="$IMAGES_DIR/rootfs.ext2"
DISK="sdb.bin"

# Verifica se os artefatos do buildroot existem
if [ ! -f "$KERNEL" ] || [ ! -f "$ROOTFS" ]; then
    echo "Erro: Imagens do kernel ou rootfs nao encontradas em $IMAGES_DIR."
    echo "Execute 'make' dentro do diretorio buildroot/ antes de iniciar a simulacao."
    exit 1
fi

# Se sdb.bin nao existir no diretorio corrente, verifica em buildroot/ ou cria
if [ ! -f "$DISK" ]; then
    if [ -f "$BUILDROOT_DIR/sdb.bin" ]; then
        DISK="$BUILDROOT_DIR/sdb.bin"
    else
        echo "Disco secundario $DISK nao encontrado. Criando imagem de 1GB..."
        ./buildroot/disk-test/create_disk.sh
        DISK="$BUILDROOT_DIR/sdb.bin"
    fi
fi

echo "Iniciando QEMU com:"
echo "  Kernel: $KERNEL"
echo "  HDA:    $ROOTFS"
echo "  HDB:    $DISK"
echo "Para sair do QEMU: pressione Ctrl+A e depois X"

if [ "$1" = "--net" ] || [ "$1" = "-n" ]; then
    echo "Modo de rede habilitado (TAP tap0)..."
    sudo qemu-system-i386 \
        --kernel "$KERNEL" \
        --hda "$ROOTFS" \
        --hdb "$DISK" \
        -net nic,model=e1000 \
        -net tap,ifname=tap0,script="$BUILDROOT_DIR/custom-scripts/qemu-ifup",downscript=no \
        --nographic \
        --append "console=ttyS0 root=/dev/sda"
else
    qemu-system-i386 \
        --kernel "$KERNEL" \
        --hda "$ROOTFS" \
        --hdb "$DISK" \
        --nographic \
        --append "console=ttyS0 root=/dev/sda"
fi
