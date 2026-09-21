# Relatórios de Desempenho e Comunicação de Rede (Tutorial 1.3)

Este diretório contém os relatórios e medições dos testes de rede realizados entre o sistema operacional guest (QEMU x86 / Buildroot) e a máquina host.

---

## Sumário dos Testes Realizados

| Teste | Ferramenta | Protocolo | Métrica / Objetivo | Resultado Obtido | Arquivo Detalhado |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **3.1. Bandwidth** | iPerf | TCP | Largura de Banda / Throughput | **952 Mbits/sec** | [relatorio_3.1_bandwidth.md](./relatorio_3.1_bandwidth.md) |
| **3.2. Jitter** | iPerf | UDP | Variação de Atraso e Perda | **0.034 ms** / **0% perda** | [relatorio_3.2_jitter.md](./relatorio_3.2_jitter.md) |
| **3.3. Comunicação** | Netcat (`nc`) | TCP | Troca de dados bidirecional na porta 8000 | **Sucesso com 100% integridade** | [relatorio_3.3_comunicacao_netcat.md](./relatorio_3.3_comunicacao_netcat.md) |

---

## Configuração do Ambiente de Testes
- **Distribuição Guest:** Linux Buildroot 2022.02 (Kernel 5.15.18, arquitetura i386)
- **Placa de Rede Emulada:** Intel(R) PRO/1000 Gigabit Ethernet (`e1000`)
- **Interface de Rede Host:** Virtual TAP (`tap0`)
- **Endereçamento IP:**
  - Guest (Target): `192.168.1.10`
  - Host: `10.0.10.153`
- **Pacotes Instalados:**
  - `iperf` (versão 2.1.6 no Host e Guest)
  - `netcat` (versão 0.7.1 no Guest e `netcat-openbsd` no Host)
  - `dropbear` (servidor SSH no Guest com login de root)

---

## Como Reproduzir os Testes

1. **Iniciar a Máquina Virtual:**
   ```bash
   cd /workspaces/labsisop-buildroot/linuxdistro/buildroot
   sudo qemu-system-i386 --device e1000,netdev=eth0,mac=aa:bb:cc:dd:ee:ff \
       --netdev tap,id=eth0,script=custom-scripts/qemu-ifup \
       --kernel output/images/bzImage \
       --hda output/images/rootfs.ext2 \
       --nographic \
       --append "console=ttyS0 root=/dev/sda"
   ```

2. **Teste 3.1 (Bandwidth - TCP):**
   - No Guest: `iperf -s`
   - No Host: `./src/iperf -c 192.168.1.10 -i 1 -t 5`

3. **Teste 3.2 (Jitter - UDP):**
   - No Guest: `iperf -s -u`
   - No Host: `./src/iperf -c 192.168.1.10 -i 1 -t 5 -u`

4. **Teste 3.3 (Comunicação - Netcat):**
   - No Guest: `nc -l -p 8000`
   - No Host: `nc 192.168.1.10 8000` (e digite o texto)
