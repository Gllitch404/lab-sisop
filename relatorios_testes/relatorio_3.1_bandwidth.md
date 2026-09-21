# Relatório de Teste 3.1 - Largura de Banda (Bandwidth - TCP)

- **Data do Teste:** 21/09/2026
- **Ambiente:** QEMU i386 (Guest / Target) ↔ Linux Host (Container Codespaces)
- **Protocolo:** TCP
- **Ferramenta:** iPerf 2.1.6
- **IP Target (Guest):** `192.168.1.10`
- **IP Host:** `10.0.10.153`

---

## 1. Descrição do Teste
O teste de largura de banda TCP avalia a taxa máxima de transferência contínua sustentada entre o host e a máquina virtual (guest) através da interface emulada Gigabit Ethernet (`e1000`) conectada via interface virtual `tap`.

---

## 2. Comandos Executados

### No Target (Servidor)
```bash
iperf -s
```

### No Host (Cliente)
```bash
./src/iperf -c 192.168.1.10 -i 1 -t 5
```

---

## 3. Saída do Servidor (Target)

```text
------------------------------------------------------------
Server listening on TCP port 5001
TCP window size:  128 KByte (default)
------------------------------------------------------------
[  1] local 192.168.1.10 port 5001 connected with 10.0.10.153 port 51426
[ ID] Interval       Transfer     Bandwidth
[  1] 0.00-5.02 sec   569 MBytes   952 Mbits/sec
```

---

## 4. Saída do Cliente (Host)

```text
------------------------------------------------------------
Client connecting to 192.168.1.10, TCP port 5001
TCP window size: 85.0 KByte (default)
------------------------------------------------------------
[  1] local 10.0.10.153 port 51426 connected with 192.168.1.10 port 5001
[ ID] Interval       Transfer     Bandwidth
[  1] 0.00-1.00 sec   113 MBytes   948 Mbits/sec
[  1] 1.00-2.00 sec   113 MBytes   948 Mbits/sec
[  1] 2.00-3.00 sec   119 MBytes   996 Mbits/sec
[  1] 3.00-4.00 sec   108 MBytes   909 Mbits/sec
[  1] 4.00-5.00 sec   116 MBytes   973 Mbits/sec
[  1] 0.00-5.03 sec   569 MBytes   950 Mbits/sec
```

---

## 5. Análise dos Resultados
- **Total Transferido:** 569 MBytes em aproximadamente 5 segundos.
- **Vazão Média (Target / Servidor):** **952 Mbits/sec**.
- **Vazão Média (Host / Cliente):** **950 Mbits/sec**.
- **Conclusão:** O canal de comunicação emulado entre host e guest atingiu praticamente o throughput máximo teórico da interface `e1000` (1 Gbps), evidenciando excelente desempenho de comunicação de rede entre a máquina hospedeira e a emulação do QEMU.

