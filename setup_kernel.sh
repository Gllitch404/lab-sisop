#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

if [ ! -d "linux-4.13.9" ]; then
    echo "Extraindo linux-4.13.9.tar.xz..."
    tar -xf linux-4.13.9.tar.xz
fi

echo "Aplicando patch do kernel (syscalls e custom defconfig)..."
cd linux-4.13.9
if patch -p1 -N --dry-run < ../kernel_syscalls.patch > /dev/null 2>&1; then
    patch -p1 < ../kernel_syscalls.patch
    echo "Patch do Kernel aplicado com sucesso!"
else
    echo "Patch já aplicado ou arquivos já configurados."
fi
