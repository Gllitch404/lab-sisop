# Contexto do Projeto: Laboratório de Sistemas Operacionais (Buildroot + Linux Kernel + QEMU)

Este arquivo serve como contexto completo para assistentes de IA (e desenvolvedores) ao abrir este projeto em um novo ambiente ou sessão de chat.

---

## 1. Visão Geral do Projeto
- **Disciplina:** Laboratório de Sistemas Operacionais (PUCRS).
- **Ambiente:** Linux x86 (Guest emulado no QEMU via Buildroot 2022.02) ↔ Host Linux/WSL2 (Ubuntu).
- **Objetivos Principais:** Customização de distribuições Linux embarcadas, implementação de chamadas de sistema (System Calls), desenvolvimento de drivers de dispositivos (Kernel Modules) e testes de rede.

---

## 2. Estrutura do Repositório
- `buildroot/`: Diretório principal do Buildroot com as configurações da distribuição.
  - `.config.tut1`: Configuração do Tutorial 1.3 (C++, iperf, netcat).
  - `.config`: Configuração atual do Buildroot.
  - `custom-scripts/`:
    - `qemu-ifup`: Configuração de rede TAP para o QEMU.
    - `S41network-config`: Script de inicialização de rede (`/etc/init.d/`).
    - `S50hello`: Script de inicialização automática da aplicação `hello` (Tutorial 1.3).
    - `syscall_test.c`: Aplicação de teste em C para as chamadas de sistema (Tutorial 2.2).
    - `pre-build.sh`: Script acionado no buildroot para copiar scripts e compilar módulos.
  - `modules/`: Módulos de kernel externos (Tutorial 2.3):
    - `hello/`: Driver básico `khello.c`.
    - `simple_driver/`: Driver de caractere com fila `list_head` do kernel.
    - `xtea_driver/`: Driver com suporte a parâmetros e algoritmo XTEA.
  - `iperf/`: Versão 2.1.6 compilada no Host para testes de throughput e jitter.
- `linux-4.13.9.tar.xz`: Tarball oficial do Kernel Linux 4.13.9.
- `kernel_syscalls.patch`: Patch contendo todas as chamadas de sistema implementadas e o `qemu_x86_custom_defconfig`.
- `setup_kernel.sh`: Script utilitário para descompactar o kernel e aplicar o patch automaticamente.
- `relatorios_testes/`: Relatórios completos em Markdown dos testes 3.1 a 3.4 do Tutorial 1.3:
  - `relatorio_3.1_bandwidth.md`: Medição de largura de banda TCP (952 Mbps).
  - `relatorio_3.2_jitter.md`: Medição de jitter UDP (0.034 ms, 0% perda).
  - `relatorio_3.3_comunicacao_netcat.md`: Troca de dados TCP na porta 8000 via Netcat.
  - `relatorio_3.4_primeira_aplicacao.md`: Cross-compilação e boot automático com `hello.c` e `S50hello`.
- `Tutorial *.html` / `.mhtml`: Cópias salvas de todos os tutoriais da disciplina (1.1, 1.2, 1.3, 2.2).

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

### ✅ Tutorial 2.2: Chamadas de Sistema (System Calls)
- Modificações no Kernel 4.13.9:
  - Criação do diretório `syscall/` no kernel.
  - Syscall `printMessage` (Desafio 2).
  - Syscall `listSleepProcesses` (Desafio 1).
  - Syscall `processInfo` (Exemplo do tutorial).
  - Tabela `arch/x86/entry/syscalls/syscall_32.tbl` atualizada com as novas syscalls.
  - Arquivo `syscall_test.c` criado em `buildroot/custom-scripts/`.
  - Todas as alterações estão salvas e consolidadas no patch `kernel_syscalls.patch`.

### ✅ Tutorial 2.3: Device Drivers (Módulos do Kernel)
- Implementação de módulos externos do kernel em `buildroot/modules/`:
  - `khello`: Módulo introdutório com `module_init` e `module_exit`.
  - `simple_driver`: Device driver de caractere com manipulação de `struct list_head` do kernel e `test_simple_driver.c`.
  - `xtea_driver`: Device driver com parâmetros de módulo (`module_param`) e `test_xtea_driver.c`.

---

## 4. Contexto da Migração (Codespaces ➔ WSL2 Local)
- O projeto foi migrado de um GitHub Codespaces (que estava atingindo o limite de 32GB de disco) para o **WSL2 (Ubuntu)** no computador local do usuário.
- Para configurar o ambiente local:
  ```bash
  sudo apt update && sudo apt install -y build-essential gcc g++ binutils patch bzip2 perl tar cpio unzip rsync bc findutils wget libncurses-dev git qemu-system-x86
  git clone https://github.com/Gllitch404/lab-sisop.git
  cd lab-sisop
  ./setup_kernel.sh
  ```
