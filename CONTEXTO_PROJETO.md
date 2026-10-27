# Contexto do Projeto: Laboratório de Sistemas Operacionais (Buildroot + Linux Kernel + QEMU)

Este arquivo serve como contexto completo para assistentes de IA (e desenvolvedores) ao abrir este projeto em um novo ambiente ou sessão de chat.

---

## 1. Visão Geral do Projeto
- **Disciplina:** Laboratório de Sistemas Operacionais (PUCRS).
- **Ambiente:** Linux x86 (Guest emulado no QEMU via Buildroot 2022.02) ↔ Host Linux/WSL2 (Ubuntu).
- **Objetivos Principais:** Customização de distribuições Linux embarcadas, implementação de chamadas de sistema (System Calls), desenvolvimento de drivers de dispositivos (Kernel Modules), escalonamento de E/S de disco (IO-Schedulers), depuração via Ftrace e implementação de novas políticas de escalonamento de CPU (`SCHED_LOW_IDLE`).

---

## 2. Estrutura do Repositório
- `buildroot/`: Diretório principal do Buildroot com as configurações da distribuição.
  - `.config`: Configuração atual do Buildroot (Kernel 4.13.9, uClibc, ext2 60MB, pre-build e post-build scripts).
  - `.config.tut1`: Configuração do Tutorial 1.3 (C++, iperf, netcat).
  - `custom-scripts/`:
    - `qemu-ifup`: Configuração de rede TAP para o QEMU.
    - `S41network-config`: Script de inicialização de rede (`/etc/init.d/`).
    - `S50hello`: Script de inicialização automática da aplicação `hello` (Tutorial 1.3).
    - `syscall_test.c`: Aplicação de teste em C para as chamadas de sistema (Tutorial 2.2).
    - `ftrace_test.sh`: Script utilitário para testes automatizados de Ftrace (Tutorial 3.1 - Ftrace).
    - `pre-build.sh`: Script acionado no buildroot para preparar fstab (`tracefs`), copiar scripts e compilar módulos.
  - `modules/`: Módulos de kernel externos (Tutorial 2.3):
    - `hello/`: Driver básico `khello.c` e Makefile.
    - `simple_driver.tar.gz`: Pacote original do driver fornecido no tutorial.
    - `simple_driver_orig/`: Cópia descompactada e intacta do driver original com `printk()`.
    - `simple_driver/`: Driver com fila encadeada `struct list_head` (Tutorial 2.3 - Atividade 1) atualizado com `trace_printk()` (Tutorial 3.1 - Ftrace Atividade 6) e `test_simple_driver.c`.
    - `xtea_driver/`: Driver com suporte a criptografia XTEA, comandos customizados e parâmetros via `modprobe` (Tutorial 2.3 - Atividade 2 e Desafio) e `test_xtea_driver.c`.
  - `disk-test/`: Utilitários de acesso direto e escalonador de disco (Tutorial 2.4):
    - `disk_test.c`: Aplicação para escrita "Hello World!" no início do disco, leitura, cálculos de setor (`pos / 512`) e drop caches.
    - `Makefile`: Compilação cruzada e instalação em `/usr/bin/disk_test`.
    - `create_disk.sh`: Script para criação do disco de 1GB (`dd if=/dev/zero of=sdb.bin bs=512 count=2097152`).
  - `setpriority/`: Utilitários de escalonamento de processos e threads (Tutorial 3.1 - Scheduler):
    - `setpriority.c`: Utilitário para consulta e definição de políticas de escalonamento por thread ou por PID (`sched_setscheduler`).
    - `deadline.c`: Exemplo de configuração de threads sob a política `SCHED_DEADLINE`.
    - `thread_runner.c`: Comparativo concorrente demonstrando a preempção entre `SCHED_NORMAL`, `SCHED_IDLE` e `SCHED_LOW_IDLE`.
    - `Makefile`: Compilação cruzada e instalação em `/usr/bin/` e `/bin/`.
  - `create_disk.sh`: Cópia do utilitário de criação do disco no diretório buildroot.
  - `iperf/`: Versão 2.1.6 compilada no Host para testes de throughput e jitter.
- `linux-4.13.9.tar.xz`: Tarball oficial do Kernel Linux 4.13.9.
- `kernel_syscalls.patch`: Patch unificado contendo:
  - Novas chamadas de sistema (385, 386, 387).
  - Nova política de escalonamento `SCHED_LOW_IDLE` (valor 7, `core.c`, `fair.c`, `sched.h`).
  - Defconfig customizado `qemu_x86_custom_defconfig` com Ftrace ativado (`CONFIG_FTRACE=y`, `CONFIG_FUNCTION_TRACER=y`, `CONFIG_FUNCTION_GRAPH_TRACER=y`, `CONFIG_DYNAMIC_FTRACE=y`, `CONFIG_DEBUG_FS=y`).
- `setup_kernel.sh`: Script utilitário para descompactar o kernel e aplicar o patch automaticamente.
- `run_qemu.sh`: Script utilitário para inicializar o QEMU com o disco secundário (`sdb.bin`) e suporte a rede.
- `relatorios_testes/`: Relatórios completos em Markdown dos testes 3.1 a 3.4 do Tutorial 1.3:
  - `relatorio_3.1_bandwidth.md`: Medição de largura de banda TCP (952 Mbps).
  - `relatorio_3.2_jitter.md`: Medição de jitter UDP (0.034 ms, 0% perda).
  - `relatorio_3.3_comunicacao_netcat.md`: Troca de dados TCP na porta 8000 via Netcat.
  - `relatorio_3.4_primeira_aplicacao.md`: Cross-compilação e boot automático com `hello.c` e `S50hello`.
- `Tutorial *.html`: Cópias salvas de todos os tutoriais da disciplina (1.3, 2.1, 2.2, 2.3, 2.4, 3.1 Ftrace, 3.1 Scheduler).

---

## 3. Status das Atividades Realizadas

### ✅ Tutorial 1.1: Buildroot e QEMU
- Configuração básica do Buildroot e geração da imagem bootável no QEMU.

### ✅ Tutorial 1.2: Configurando a Rede
- Configuração de bridge/TAP host-guest, endereçamento IP (`192.168.1.10`) e script `qemu-ifup`.

### ✅ Tutorial 1.3: Iperf, Netcat e Primeira Aplicação
- Suporte C++ na toolchain do Buildroot e ativação do `iperf` e `netcat`.
- Driver `e1000` (Intel PRO/1000) habilitado no kernel.
- Compilação nativa do Iperf 2.1.6 no Host.
- Execução e relatórios de Bandwidth (3.1), Jitter (3.2), Netcat (3.3).
- Aplicação `hello` cross-compilada para x86 32-bit e script SysV `S50hello` adicionado.

### ✅ Tutorial 2.1: Obtenção e Configuração do Kernel 4.13.9
- Download do kernel `linux-4.13.9.tar.xz`.
- Configuração do Buildroot (`qemu_x86_defconfig`, Kernel 4.13.9, Headers 4.13.x, DHCP desativado).
- Arquivo `qemu_x86_custom_defconfig` criado e configurado como defconfig in-tree.
- Variável `LINUX_OVERRIDE_SRCDIR` suportada para compilação local rápida.

### ✅ Tutorial 2.2: Chamadas de Sistema (System Calls)
- Modificações no Kernel 4.13.9:
  - Criação do diretório `syscall/` no kernel com `processInfo.c`, `sleepProcesses.c`, `printMessage.c` e `Makefile`.
  - Syscall 385: `listProcessInfo` (Exemplo do tutorial).
  - Syscall 386: `listSleepProcesses` (Desafio 1 - processos em `TASK_INTERRUPTIBLE` e `TASK_UNINTERRUPTIBLE`).
  - Syscall 387: `printMessage` (Desafio 2 - imprime mensagem enviada do espaço de usuário no dmesg do kernel).
  - Tabela `arch/x86/entry/syscalls/syscall_32.tbl` e `include/linux/syscalls.h` atualizadas.
  - Aplicação `syscall_test.c` criada em `buildroot/custom-scripts/` exercitando as chamadas 385, 386 e 387.
  - Script `board/qemu/x86/post-build.sh` compila e instala automaticamente `syscall_test` em `/bin/syscall_test`.

### ✅ Tutorial 2.3: Device Drivers (Módulos do Kernel)
- `khello` (Seção 1.1): Módulo introdutório com `module_init` e `module_exit`.
- `simple_driver.tar.gz`: Arquivo original arquivado e preservado em `buildroot/modules/`.
- `simple_driver` (Atividade 1): Implementação de fila com `struct list_head`, inserção em `write()` e remoção FIFO em `read()`, com programa de teste `test_simple_driver`.
- `xtea_driver` (Atividade 2 & Desafio): Driver de criptografia em `/dev/xtea_driver` implementando algoritmo XTEA de 64 bits/128 bits com 32 rounds, suporte ao protocolo em texto (`enc/dec <k0> <k1> <k2> <k3> <size> <hex>`), suporte a parâmetros de módulo via `modprobe xtea_driver key0=...` e programa de teste `test_xtea_driver`.
- Automação via `pre-build.sh` e `post-build.sh` com compatibilidade de toolchain.

### ✅ Tutorial 2.4: IO-Scheduler e Acesso a Disco
- Seção 2: Criação de imagem de teste de 1GB via script `create_disk.sh` (`dd if=/dev/zero of=sdb.bin bs=512 count=2097152`). Adicionado `sdb.bin` e `*.bin` ao `.gitignore` para não sobrecarregar o git.
- Seção 3: Adição do disco no QEMU via parâmetro `--hdb sdb.bin`, mapeado como `/dev/sdb` no guest. Script `run_qemu.sh` adicionado para executar o comando completo do tutorial.
- Seção 4: Diretório `buildroot/disk-test/` criado com `disk_test.c` e `Makefile`:
  - Execução padrão sem argumentos executa a escrita literal de `"Hello World!"` no offset 0 de `/dev/sdb`, verificável na máquina host via `hexdump -C sdb.bin`.
  - Suporte a leitura direta (`disk_test read`), escrita arbitrária (`disk_test write`), cálculo de setores (`disk_test sector`) e escrita em setores específicos (`disk_test writesec`).
  - Compilação automatizada via `post-build.sh` com instalação em `/usr/bin/disk_test` e `/bin/disk_test`.
- Seção 4.1: Utilitário para limpeza de cache de disco (`echo 3 > /proc/sys/vm/drop_caches`).
- Seção 5: Instruções e suporte para verificação e chaveamento dinâmico de escalonadores (`cat /sys/block/sdb/queue/scheduler`, `echo noop > /sys/block/sdb/queue/scheduler`).

### ✅ Tutorial 3.1: Depurando o Kernel com Ftrace (Parte 1)
- Seção 2: Ftrace habilitado no kernel (`CONFIG_FTRACE=y`, `CONFIG_FUNCTION_TRACER=y`, `CONFIG_FUNCTION_GRAPH_TRACER=y`, `CONFIG_DYNAMIC_FTRACE=y`, `CONFIG_DEBUG_FS=y`).
- Seção 2: Montagem de `tracefs` configurada no `/etc/fstab` (`tracefs /sys/kernel/tracing tracefs 0 0`) através dos scripts de build.
- Seções 3-5: Suporte a `function tracer`, `function_graph` e controle de ativação via `tracing_on`. Script `ftrace_test.sh` criado e instalado em `/usr/bin/ftrace_test`.
- Seção 6 (Atividade): Módulo `simple_driver` atualizado para substituir chamadas a `printk()` por `trace_printk()`, enviando mensagens diretamente ao buffer de trace.

### ✅ Tutorial 3.1: Linux Scheduler - SCHED_LOW_IDLE
- Seção 3.1: Política `SCHED_LOW_IDLE` com ID 7 adicionada em `include/uapi/linux/sched.h`.
- Seção 3.2: `kernel/sched/sched.h` modificado com `idle_low_policy()`, `valid_policy()` atualizado, `WEIGHT_LOW_IDLEPRIO 1` e `WMULT_LOW_IDLEPRIO 1431655765`.
- Seção 3.3: `kernel/sched/core.c` atualizado em `set_load_weight()`, `__sched_setscheduler()` (restrição de nice) e limites de prioridade (min 0, max 0 em `sched_get_priority_min` e `sched_get_priority_max`).
- Seção 3.4: `kernel/sched/fair.c` atualizado em `set_last_buddy()`, `set_next_buddy()`, `task_hot()` e `check_preempt_wakeup()` (garantindo que `SCHED_LOW_IDLE` não preempte `SCHED_IDLE`, mas seja preemptado por todas as outras classes).
- Seção 4: Implementação dos utilitários em `setpriority/` (`setpriority`, `deadline`, `thread_runner`), instalados automaticamente no rootfs em `/usr/bin/`.

---

## 4. Instruções de Compilação e Execução no Host (WSL2 / Linux)

```bash
# 1. Instalar dependências no Ubuntu/WSL2
sudo apt update && sudo apt install -y build-essential gcc g++ binutils patch bzip2 perl tar cpio unzip rsync bc findutils wget libncurses-dev git qemu-system-x86

# 2. Configurar o kernel com as syscalls, escalonador SCHED_LOW_IDLE e Ftrace
./setup_kernel.sh

# 3. Compilar a distribuição Buildroot
cd buildroot
export LINUX_OVERRIDE_SRCDIR="$(pwd)/../linux-4.13.9"
make

# 4. Criar o disco secundário sdb.bin (1GB)
./create_disk.sh

# 5. Executar o QEMU com o disco
cd ..
./run_qemu.sh
```
