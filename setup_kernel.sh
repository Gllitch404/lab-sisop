#!/bin/bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

if [ ! -d "linux-4.13.9" ]; then
    echo "Extraindo linux-4.13.9.tar.xz..."
    tar -xf linux-4.13.9.tar.xz
fi

# Remove a pasta de versionamento (.git) do kernel se existir, conforme orientação
# do professor para economizar centenas de MB / GB no Codespaces.
if [ -d "linux-4.13.9/.git" ]; then
    echo "Removendo pasta .git pesada de dentro de linux-4.13.9..."
    rm -rf linux-4.13.9/.git
fi

echo "Aplicando patch do kernel (syscalls, SCHED_LOW_IDLE, ftrace e custom defconfig)..."
cd linux-4.13.9
if patch -p1 -N --dry-run < ../kernel_syscalls.patch > /dev/null 2>&1; then
    patch -p1 < ../kernel_syscalls.patch
    echo "Patch do Kernel aplicado com sucesso!"
else
    echo "Patch ja aplicado ou arquivos ja configurados."
fi
