# Relatório de Teste 3.2 - Jitter e Perda de Datagramas (UDP)

- **Data do Teste:** 21/09/2026
- **Ambiente:** QEMU i386 (Guest / Target) ↔ Linux Host (Container Codespaces)
- **Protocolo:** UDP
- **Ferramenta:** iPerf 2.1.6
- **IP Target (Guest):** `192.168.1.10`
- **IP Host:** `10.0.10.153`

---

## 1. Descrição do Teste
O jitter representa a variação estatística no atraso de entrega de pacotes na rede (latência). Para esta medição, utiliza-se o protocolo UDP com datagramas de tamanho padronizado (1470 bytes) e taxa constante, observando:
1. Variação do atraso (*Jitter* em milissegundos).
2. Perda de datagramas (*Packet Loss*).

---

## 2. Comandos Executados

### No Target (Servidor UDP)
```bash
iperf -s -u
```

### No Host (Cliente UDP)
```bash
./src/iperf -c 192.168.1.10 -i 1 -t 5 -u
```

---

## 3. Saída do Servidor (Target)

```text
------------------------------------------------------------
Server listening on UDP port 5001
UDP buffer size:  176 KByte (default)
------------------------------------------------------------
[  1] local 192.168.1.10 port 5001 connected with 10.0.10.153 port 39363
[ ID] Interval       Transfer     Bandwidth        Jitter   Lost/Total Datagrams
[  1] 0.00-5.01 sec   645 KBytes  1.05 Mbits/sec   0.034 ms 0/449 (0%)
```

---

## 4. Saída do Cliente (Host)

```text
------------------------------------------------------------
Client connecting to 192.168.1.10, UDP port 5001
Sending 1470 byte datagrams, IPG target: 11215.21 us (kalman adjust)
UDP buffer size:  208 KByte (default)
------------------------------------------------------------
[  1] local 10.0.10.153 port 39363 connected with 192.168.1.10 port 5001
[ ID] Interval       Transfer     Bandwidth
[  1] 0.00-1.00 sec   131 KBytes  1.07 Mbits/sec
[  1] 1.00-2.00 sec   128 KBytes  1.05 Mbits/sec
[  1] 2.00-3.00 sec   128 KBytes  1.05 Mbits/sec
[  1] 3.00-4.00 sec   128 KBytes  1.05 Mbits/sec
[  1] 4.00-5.00 sec   128 KBytes  1.05 Mbits/sec
[  1] 0.00-5.01 sec   645 KBytes  1.05 Mbits/sec
[  1] Sent 450 datagrams
[  1] Server Report:
[ ID] Interval       Transfer     Bandwidth        Jitter   Lost/Total Datagrams
[  1] 0.00-5.01 sec   645 KBytes  1.05 Mbits/sec   0.033 ms 0/449 (0%)
```

---

## 5. Análise dos Resultados
- **Total de Datagramas Enviados:** 450 datagramas (645 KBytes).
- **Datagramas Recebidos com Sucesso:** 449 datagramas.
- **Perda de Pacotes:** **0%** (0 perdas registradas).
- **Jitter Medido no Servidor:** **0.034 ms** (34 microssegundos).
- **Vazão UDP:** **1.05 Mbits/sec** (taxa padrão de envio UDP do iPerf2).
- **Conclusão:** O canal de comunicação emulado apresentou estabilidade com variação de atraso (jitter) desprezível de ~0.034 ms e índice nulo de perdas de pacotes.

