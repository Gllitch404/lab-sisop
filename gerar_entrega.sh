#!/bin/bash
# Script utilitário para gerar o arquivo de entrega sched_profiler.zip

set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

echo "=== Gerando pacote de entrega: sched_profiler.zip ==="

# Verifica se o utilitário zip está disponível
if ! command -v zip >/dev/null 2>&1; then
    sudo apt-get update && sudo apt-get install -y zip
fi

# Limpa build anterior se existir
rm -f sched_profiler.zip

# Cria o arquivo zip contendo o código fonte .c, README.md, Makefile e a pasta de imagens
cd sched_profiler
zip -r ../sched_profiler.zip sched_profiler.c README.md Makefile imagens/
cd ..

echo "Pacote gerado com sucesso!"
ls -lh sched_profiler.zip
