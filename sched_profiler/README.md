# Laboratório de Sistemas Operacionais: sched_profiler

## 1. Descrição do Trabalho
Este projeto implementa a aplicação multithread **`sched_profiler`**, desenvolvida para a disciplina de Laboratório de Sistemas Operacionais (Ciência da Computação - PUCRS).

O objetivo é avaliar empiricamente as características de escalonamento de processos no Linux (CFS, políticas de tempo real e políticas idle customizadas), observando a alternância de threads e o impacto de diferentes políticas em ambientes monoprocessados (mono-core) e multiprocessados.

---

## 2. Arquitetura da Aplicação

1. **Buffer Global e Ponteiro:**
   - A aplicação aloca dinamicamente um buffer contínuo em memória (`char *global_buffer`) com o tamanho especificado por parâmetro (`buffer_size`).
   - Um índice/ponteiro global compartilhado (`int global_index`) controla a posição corrente de escrita.

2. **Sincronização com Semáforo Binário:**
   - O acesso concorrente ao buffer e a atualização do ponteiro global são sincronizados estritamente através de um semáforo binário (`sem_t sem_buffer`), inicializado via `sem_init(&sem_buffer, 0, 1)`.
   - Cada thread executa `sem_wait(&sem_buffer)`, grava seu caractere identificador (`'A' + thread_id`), incrementa o índice global e libera a seção crítica através de `sem_post(&sem_buffer)`.

3. **Políticas de Escalonamento Suportadas:**
   - `SCHED_FIFO` (1): Tempo real (First In, First Out). Sem time-slice; a thread roda até terminar ou bloquear.
   - `SCHED_RR` (2): Tempo real com Round-Robin e quantum fixo de tempo.
   - `SCHED_OTHER` / `SCHED_NORMAL` (0): Escalonador padrão CFS do Linux.
   - `SCHED_IDLE` (5): Escalonador para tarefas de prioridade muito baixa (CFS weight 3).
   - `SCHED_LOW_IDLE` (7): Política customizada desenvolvida no Kernel 4.13.9 da disciplina (CFS weight 1), preemptada por todas as demais classes.

---

## 3. Parâmetros de Entrada e Execução

### Sintaxe
```bash
./sched_profiler <buffer_size> <num_threads> <policy>
```

- `<buffer_size>`: Quantidade total de caracteres a serem escritos no buffer global (ex.: `10000`).
- `<num_threads>`: Quantidade de threads simultâneas a serem criadas (ex.: `4`).
- `<policy>`: Política de escalonamento a ser configurada nas threads (`SCHED_LOW_IDLE`, `SCHED_IDLE`, `SCHED_FIFO`, `SCHED_RR` ou `SCHED_OTHER`).

### Exemplos de Execução

#### No Host (forçando um único núcleo via taskset):
```bash
taskset -c 0 ./sched_profiler 10000 4 SCHED_RR
```

#### No Guest (QEMU emulando monoprocessador `--smp cpus=1`):
```bash
./sched_profiler 10000 4 SCHED_FIFO
./sched_profiler 10000 4 SCHED_RR
./sched_profiler 10000 4 SCHED_IDLE
./sched_profiler 10000 4 SCHED_LOW_IDLE
```

---

## 4. Exemplo de Saída do Programa

### Comando executado:
```bash
./sched_profiler 10000 4 SCHED_RR
```

### Saída gerada:
```text
============================================================
   Laboratorio de Sistemas Operacionais - sched_profiler   
============================================================
Configuracao:
  Tamanho do Buffer:  10000 bytes
  Numero de Threads:  4
  Politica Escolhida: SCHED_RR (2)
============================================================

Saida sem pos-processamento:
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABBBBBBBBBBBBBBB
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
BBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDDDDDDD
DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
DDDDDDDDDDDDDDDDDAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
...

Saida apos pos-processamento:
ABCDABCDABCDABCD
A = 4
B = 4
C = 4
D = 4
```

> **Explicação do Pós-processamento:**
> O pós-processador agrega caracteres consecutivos idênticos, evidenciando as fatias de tempo em que cada thread manteve o controle da CPU. O contador quantifica quantas vezes cada thread obteve o processador (número de vezes escalonada).

---

## 5. Captura com trace-cmd e Visualização no KernelShark

### 5.1. Capturando eventos no Guest (QEMU)
No QEMU (iniciado com `./run_qemu.sh` que já define `-smp cpus=1`):
```bash
# Exemplo para SCHED_RR:
trace-cmd record -e sched:sched_switch -o /root/trace_rr.dat ./sched_profiler 10000 4 SCHED_RR

# Exemplo para SCHED_FIFO:
trace-cmd record -e sched:sched_switch -o /root/trace_fifo.dat ./sched_profiler 10000 4 SCHED_FIFO

# Exemplo para SCHED_IDLE:
trace-cmd record -e sched:sched_switch -o /root/trace_idle.dat ./sched_profiler 10000 4 SCHED_IDLE

# Exemplo para SCHED_LOW_IDLE:
trace-cmd record -e sched:sched_switch -o /root/trace_low_idle.dat ./sched_profiler 10000 4 SCHED_LOW_IDLE
```

### 5.2. Exportando o arquivo .dat para o Host no Codespace
No terminal do Codespace (fora do QEMU), utilize `e2cp`:
```bash
# Copia o trace gerado:
e2cp buildroot/output/images/rootfs.ext2:/root/trace_rr.dat trace_rr.dat
e2cp buildroot/output/images/rootfs.ext2:/root/trace_fifo.dat trace_fifo.dat
```

### 5.3. Abrindo o KernelShark no Codespace
Para iniciar a interface gráfica do KernelShark no navegador:
```bash
curl -fsSL https://gist.githubusercontent.com/miguelxvr/3c123ef67611652af9d2e45eccb8fc0a/raw/ | bash -s -- start
```
Para encerrar a sessão:
```bash
curl -fsSL https://gist.githubusercontent.com/miguelxvr/3c123ef67611652af9d2e45eccb8fc0a/raw/ | bash -s -- stop
```

---

## 6. Análise Comparativa das Políticas no KernelShark

| Política | Comportamento Observado | Característica no KernelShark |
| :--- | :--- | :--- |
| **SCHED_FIFO** | A primeira thread que adquire o semáforo executa continuamente sem preempção por timer até que encerre ou sofra bloqueio estrito. | Longos blocos contínuos da mesma CPU para uma única thread antes de alternar. |
| **SCHED_RR** | Cada thread consome um quantum de tempo e sofre preempção periódica pelo escalonador em rodízio circular. | Alternância balanceada em fatias de tempo homogêneas entre as threads ativas. |
| **SCHED_IDLE** | Tarefas operam sob o CFS mas com peso estático 3, sendo preemptadas por qualquer outra tarefa do sistema. | Trocas de contexto frequentes compartilhando o tempo de processamento ocioso. |
| **SCHED_LOW_IDLE** | Política com peso 1 desenvolvida no kernel da disciplina, preemptada até mesmo por tarefas `SCHED_IDLE`. | Execução somente quando não houver qualquer outra demanda de processamento. |

---

## 7. Estrutura do Pacote de Entrega (`sched_profiler.zip`)
- `sched_profiler.c`: Código fonte C com implementação do semáforo binário e configuração de políticas.
- `Makefile`: Script de compilação com suporte a flags `-Wall -O2 -pthread`.
- `README.md`: Este relatório detalhado de instruções, parâmetros e saídas.
- `imagens/`: Capturas de tela dos traces gerados no KernelShark para cada política.
