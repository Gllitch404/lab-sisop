#!/bin/bash
# Script utilitário para liberar espaço no Codespaces
# Remove pastas pesadas de versionamento do kernel, saídas de compilação e discos temporários

set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

echo "=== Liberando espaço no Codespaces ==="

# 1. Pasta .git pesada do kernel (conforme orientação do professor no Tutorial 2.1)
if [ -d "linux-4.13.9/.git" ]; then
    echo "-> Removendo versionamento local (.git) de linux-4.13.9 (~800MB a 1GB)..."
    rm -rf linux-4.13.9/.git
fi

# 2. Pasta de compilação do Buildroot (pode chegar a 5GB - 8GB)
if [ -d "buildroot/output" ]; then
    echo "-> Removendo pasta buildroot/output/ (saída pesada de compilação)..."
    rm -rf buildroot/output
fi

# 3. Discos secundários de teste do QEMU (1GB cada, Tutorial 2.4)
if [ -f "sdb.bin" ]; then
    echo "-> Removendo sdb.bin (1GB)..."
    rm -f sdb.bin
fi
if [ -f "buildroot/sdb.bin" ]; then
    echo "-> Removendo buildroot/sdb.bin (1GB)..."
    rm -f buildroot/sdb.bin
fi

# 4. Tarballs e traces temporários
if [ -f "buildroot-2022.02.tar.gz" ]; then
    echo "-> Removendo tarball redundante buildroot-2022.02.tar.gz..."
    rm -f buildroot-2022.02.tar.gz
fi
if [ -f "trace.dat" ]; then
    echo "-> Removendo trace.dat..."
    rm -f trace.dat
fi
rm -rf tracing/ 2>/dev/null || true

# 5. Binários e módulos compilados temporários
echo "-> Limpando objetos intermediários compilados..."
find . -name "*.o" -o -name "*.ko" -o -name "*.mod.c" -o -name ".tmp_versions" | xargs rm -rf 2>/dev/null || true

echo "=== Limpeza concluída com sucesso! ==="
df -h "$DIR"
