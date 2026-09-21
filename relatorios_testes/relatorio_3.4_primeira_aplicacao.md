# Relatório de Teste 3.4 - Primeira Aplicação (Cross-Compilação e Inicialização Automática)

- **Data do Teste:** 21/09/2026
- **Ambiente:** Linux Host (x86_64) ↔ QEMU Target (i386)
- **Compilador Cruzado:** `i586-linux-gcc` (GCC 10.3.0 / uClibc)
- **Aplicativo:** `hello` (`hello.c`)

---

## 1. Descrição do Teste
Construção e validação de um programa em linguagem C desenvolvido na máquina hospedeira e cross-compilado para a arquitetura alvo (x86 32-bit do Buildroot). O binário é integrado ao sistema de arquivos do target em `/usr/bin/hello` e configurado com um script de inicialização do SysV (`/etc/init.d/S50hello`) para ser executado automaticamente no boot do Linux.

---

## 2. Passo a Passo Executado

### 2.1. Configuração da variável PATH
Disponibilizado o compilador cruzado na variável PATH (e adicionado ao `~/.bashrc`):
```bash
export PATH=$PATH:/workspaces/labsisop-buildroot/linuxdistro/buildroot/output/host/bin
```

### 2.2. Criação do código-fonte (`hello.c`)
```c
#include <stdio.h>

int main(void){
    printf("Hello World!\n");
    return 0;
}
```

### 2.3. Teste com compilador nativo do Host
```bash
gcc hello.c -O2 -o hello_native
./hello_native
```
*Saída:* `Hello World!`

### 2.4. Cross-compilação para o Target
```bash
i586-linux-gcc hello.c -O2 -o hello
```
*Inspeção do binário gerado:*
```text
hello: ELF 32-bit LSB pie executable, Intel 80386, version 1 (SYSV), dynamically linked, interpreter /lib/ld-uClibc.so.0, not stripped
```

### 2.5. Instalação no Target e Script de Inicialização
1. Binário copiado para `/usr/bin/hello` no target:
   ```bash
   cp hello output/target/usr/bin/hello
   chmod +x output/target/usr/bin/hello
   ```
2. Criado o script `/etc/init.d/S50hello`:
   ```sh
   #!/bin/sh
   case "$1" in
       start)
           /usr/bin/hello
           ;;
       stop)
           exit 1
           ;;
       *)
           exit 1
           ;;
   esac

   exit 0
   ```
   Com permissão de execução: `chmod +x output/target/etc/init.d/S50hello`.

3. Recompilação do sistema de arquivos rootfs:
   ```bash
   make MAKEINFO=false
   ```

---

## 3. Validação da Execução

### 3.1. Execução Automática no Boot (Logs do QEMU)
Durante a inicialização do sistema no QEMU, os scripts em `/etc/init.d/` foram acionados sequencialmente:
```text
Starting syslogd: OK
Starting klogd: OK
Running sysctl: OK
Saving random seed: OK
Starting network: OK
Configuring host communication.e1000: eth0 NIC Link is Up 1000 Mbps Full Duplex, Flow Control: RX
IPv6: ADDRCONF(NETDEV_CHANGE): eth0: link becomes ready
OK
Starting dropbear sshd: OK
Hello World!

Welcome to Buildroot
buildroot login:
```
*A mensagem `Hello World!` foi emitida automaticamente antes da tela de login.*

### 3.2. Execução Manual via Shell no Guest
```bash
/usr/bin/hello
```
*Saída:*
```text
Hello World!
```

---

## 4. Conclusão
O fluxo de desenvolvimento cruzado (cross-compilation) funcionou perfeitamente:
- A ferramenta `i586-linux-gcc` gerou binários ELF de 32-bits compatíveis com a biblioteca `uClibc` da distribuição.
- O executável funcionou tanto na inicialização via SysV init script (`S50hello`) quanto na chamada direta por linha de comando.

