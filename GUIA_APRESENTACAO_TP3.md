# Guia Completo de Apresentação: TP3 - sched_profiler
**Laboratório de Sistemas Operacionais - PUCRS**

---

## 1. O que é o projeto (Visão Geral)
O **`sched_profiler`** é uma aplicação multithread em linguagem C desenvolvida para analisar empiricamente como o escalonador do Linux distribui o tempo de CPU entre diferentes threads concorrentes sob variadas políticas de escalonamento.

### Principais Características da Arquitetura:
1. **Buffer Compartilhado:** Um vetor contínuo de caracteres (`char *global_buffer`) alocado dinamicamente (padrão: 10.000 posições).
2. **Sincronização com Semáforo Binário:** O acesso à região crítica (gravação do caractere e avanço do índice `global_index`) é protegido estritamente por `sem_t sem_buffer` (`sem_wait` e `sem_post`).
3. **Largada Simultânea (Barreira):** Uso de `pthread_barrier_wait` para que as threads A, B, C e D só comecem a disputar a CPU após todas terem sido devidamente instanciadas.
4. **Ambiente de Teste:** Executado em ambiente estritamente monoprocessado (QEMU com `-smp cpus=1` ou host via `taskset -c 0`) para evidenciar a preempção real do escalonador.
5. **Pós-Processamento Integrado:** Agrega rajadas consecutivas do mesmo caractere para quantificar quantas vezes cada thread assumiu a CPU.
6. **Integração com Kernel (trace-cmd):** Suporte à flag `--record` para disparar diretamente a captura de eventos de troca de contexto do kernel (`sched:sched_switch`).

---

## 2. As 4 Políticas de Escalonamento e o que os Gráficos Provam

| Política | O que é? | Comportamento no Programa | O que você vê no KernelShark? |
| :--- | :--- | :--- | :--- |
| **`SCHED_FIFO`** *(1)* | Tempo Real: First In, First Out | A primeira thread que adquire o semáforo roda continuamente até terminar todas as suas operações ou bloquear. | **Barra roxa contínua** na CPU 0 (PID 100), sem preempção periódica por relógio (*timer tick*). |
| **`SCHED_RR`** *(2)* | Tempo Real: Round-Robin com fatias fixas | As threads alternam a execução ciclicamente a cada expiração do seu *time slice* (quantum). | Alternância homogênea e periódica de blocos de tempo iguais entre as 4 threads. |
| **`SCHED_IDLE`** *(5)* | CFS com prioridade baixa (Nice estático) | Opera sob o Completely Fair Scheduler (CFS), mas com peso estático baixo (weight 3). | Execução regular, mas é prontamente preemptada sempre que qualquer processo comum solicita a CPU. |
| **`SCHED_LOW_IDLE`** *(7)* | Política customizada criada no Kernel | Política desenvolvida no kernel 4.13.9 da disciplina com peso mínimo (weight 1), abaixo até do `SCHED_IDLE`. | Executa somente quando a CPU está completamente ociosa; preemptada por todas as demais classes. |

---

## 3. Roteiro de Fala para a Apresentação (Pitch de 2 Minutos)

> **Você:**
> *"Professor, o objetivo do nosso TP3 foi construir uma ferramenta de perfilamento de escalonamento chamada `sched_profiler` e validar o comportamento do kernel através de instrumentação com `trace-cmd` e `KernelShark`.*
>
> *Nossa aplicação cria 4 threads concorrentes (A, B, C e D) que disputam a escrita em um buffer global compartilhado de 10.000 posições. O acesso ao índice e ao buffer é protegido por um semáforo binário.*
>
> *Para garantir que as threads compitam de forma justa e sob o escalonamento do kernel, utilizamos uma barreira `pthread_barrier_t` na largada e uma carga natural de processamento após a liberação do semáforo, permitindo que o timer do kernel atue.*
>
> *Para analisar os resultados, criamos um algoritmo de pós-processamento que condensa repetições consecutivas e quantifica o número de rajadas de CPU de cada thread. Além disso, adicionamos a opção `--record`, que aciona de forma transparente o `trace-cmd record -e sched:sched_switch`, gravando os eventos do kernel.*
>
> *Nos gráficos do KernelShark, comprovamos as propriedades teóricas de cada política: no `SCHED_FIFO`, vimos que a tarefa monopoliza a CPU em blocos contínuos sem preempção por timer; no `SCHED_RR`, observamos o fatiamento cíclico e uniforme do tempo; e nas políticas `SCHED_IDLE` e `SCHED_LOW_IDLE`, verificamos o escalonamento pelo CFS com pesos reduzidos."*

---

## 4. Possíveis Perguntas do Professor e Respostas na Ponta da Língua

### Pergunta 1: *"Por que vocês usaram um semáforo binário e não um mutex?"*
> **Resposta:** *"O enunciado do trabalho solicitava explicitamente a sincronização das threads através de um semáforo binário da biblioteca POSIX (`sem_t`), inicializado com valor 1 (`sem_init(&sem_buffer, 0, 1)`). Ele garante a exclusão mútua na escrita do buffer e no incremento do ponteiro compartilhado."*

### Pergunta 2: *"O que é o pós-processamento que vocês fizeram?"*
> **Resposta:** *"Sem pós-processamento, a saída seria uma cadeia bruta de 10.000 letras (ex: `AAAAA...BBBBB...`). O pós-processador percorre o buffer e condensa caracteres repetidos adjacentes. Se a saída bruta tiver 500 'A's seguidos e depois 500 'B's, o pós-processamento exibe `AB`, e incrementa o contador de rajadas de A em 1 e de B em 1. Isso mostra exatamente as fatias de tempo em que cada thread reteve a CPU."*

### Pergunta 3: *"Por que precisou rodar em uma única CPU (`-smp cpus=1`)?"*
> **Resposta:** *"Se executássemos em múltiplos núcleos, as 4 threads rodariam em paralelo real, cada uma em uma CPU física diferente, e não veríamos a disputa pelo escalonamento temporal. Com apenas 1 CPU, o kernel é obrigado a alternar e decidir quem ganha a CPU a cada momento."*

### Pergunta 4: *"O que significa o parâmetro `-e sched:sched_switch` no trace-cmd?"*
> **Resposta:** *"É o evento de rastreamento do ftrace do kernel que monitora a troca de contexto (*context switch*). Ele registra no buffer do kernel exatamente quando uma tarefa sai da CPU e outra assume, com precisão de nanossegundos."*

---

## 5. Como Demonstrar na Prática Durante a Apresentação

Se o professor pedir para ver rodando:

```bash
# Execução padrão limpa (exibe apenas o pós-processamento):
sched_profiler 10000 4 SCHED_RR

# Se o professor quiser ver o buffer bruto:
sched_profiler 10000 4 SCHED_RR --showpre

# Se o professor pedir para gerar o trace ao vivo:
sched_profiler 10000 4 SCHED_RR --record
```
