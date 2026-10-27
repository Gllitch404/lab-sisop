#!/bin/sh
# Tutorial 3.1: Script utilitário para testes de Ftrace

TRACE_DIR="/sys/kernel/tracing"
if [ ! -d "$TRACE_DIR" ]; then
    TRACE_DIR="/sys/kernel/debug/tracing"
fi

if [ ! -d "$TRACE_DIR" ]; then
    echo "Montando tracefs em /sys/kernel/tracing..."
    mkdir -p /sys/kernel/tracing
    mount -t tracefs none /sys/kernel/tracing
    TRACE_DIR="/sys/kernel/tracing"
fi

cd "$TRACE_DIR" || exit 1

echo "=========================================="
echo "    TESTE DO FTRACE (Tutorial 3.1)       "
echo "=========================================="
echo "Tracers disponiveis:"
cat available_tracers

TRACER="${1:-function}"

echo ""
echo "1. Desabilitando tracing temporariamente..."
echo 0 > tracing_on

echo "2. Selecionando tracer '$TRACER'..."
echo "$TRACER" > current_tracer

echo "3. Limpando buffer de trace..."
echo > trace

echo "4. Habilitando trace, gerando atividade e parando trace..."
echo 1 > tracing_on
# Executa pequena atividade (se o simple_driver estiver carregado, escreve nele)
if [ -c /dev/simple_driver ]; then
    echo "Testando trace_printk no simple_driver..." > /dev/simple_driver 2>/dev/null || true
    cat /dev/simple_driver > /dev/null 2>&1 || true
else
    ls / > /dev/null
fi
echo 0 > tracing_on

echo "5. Exibindo as primeiras 30 linhas do arquivo trace:"
echo "---------------------------------------------------"
head -n 30 trace
echo "---------------------------------------------------"
echo "Trace concluido com sucesso!"
