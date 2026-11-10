#!/bin/sh
# Script utilitário para automação das Atividades 1 e 2 do Tutorial 3.2
# Executado dentro da máquina guest (QEMU)

echo "=========================================================="
echo " Laboratorio de SisOp - Testes com trace-cmd (Tutorial 3.2)"
echo "=========================================================="
echo "Selecione o teste a executar:"
echo "1) Atividade 1: Rastrear Chamadas de Sistema (syscall_test)"
echo "2) Atividade 2: Rastrear Troca de Contexto de Threads (sched_switch)"
echo "3) Ver relatorio do ultimo trace (/root/trace.dat)"
echo "4) Sair"
read -p "Opcao: " opt

case "$opt" in
    1)
        echo "Gravando trace da Atividade 1 (syscall_test)..."
        cd /root
        trace-cmd record -p function_graph \
            -l sys_listProcessInfo \
            -l sys_listSleepProcesses \
            -l sys_printMessage \
            -o /root/trace.dat \
            syscall_test
        echo "Trace gravado em /root/trace.dat!"
        echo "Use 'trace-cmd report' para visualizar ou exporte para o host."
        ;;
    2)
        echo "Iniciando threads_test em segundo plano..."
        cd /root
        threads_test > /dev/null 2>&1 &
        TPID=$!
        echo "Gravando eventos sched_switch por 3 segundos..."
        trace-cmd record -e sched_switch -o /root/trace.dat sleep 3
        kill -9 $TPID 2>/dev/null || true
        echo "Trace gravado em /root/trace.dat!"
        echo "Total de trocas de contexto registradas:"
        trace-cmd report | grep "threads_test" | head -n 15
        echo "..."
        echo "Use 'trace-cmd report' para visualizar ou exporte para o host."
        ;;
    3)
        if [ -f /root/trace.dat ]; then
            trace-cmd report /root/trace.dat | head -n 50
        else
            echo "Arquivo /root/trace.dat nao encontrado!"
        fi
        ;;
    *)
        echo "Saindo."
        ;;
esac
