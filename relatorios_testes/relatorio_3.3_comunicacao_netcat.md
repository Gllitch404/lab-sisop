# Relatório de Teste 3.3 - Comunicação com Netcat (TCP)

- **Data do Teste:** 21/09/2026
- **Ambiente:** QEMU i386 (Guest / Target) ↔ Linux Host (Container Codespaces)
- **Protocolo:** TCP
- **Ferramenta:** GNU Netcat 0.7.1
- **Porta utilizada:** 8000
- **IP Target (Guest):** `192.168.1.10`
- **IP Host:** `10.0.10.153`

---

## 1. Configuração Prévia no Buildroot
Conforme indicado no tutorial, para disponibilizar o comando `nc` (netcat) no target:
- No `make menuconfig`:
  - `Target packages --->`
    - `[*] Show packages that are also provided by busybox` (`BR2_PACKAGE_BUSYBOX_SHOW_OTHERS=y`)
    - `Networking applications --->`
      - `[*] netcat` (`BR2_PACKAGE_NETCAT=y`)
- O pacote foi compilado e instalado no sistema de arquivos do Guest em `/usr/bin/netcat` (com link simbólico `/usr/bin/nc`).
- No Host, foi instalado o pacote `netcat-openbsd`.

---

## 2. Comandos Executados

### No Target (Escuta / Servidor)
Executou-se o `nc` em modo de escuta na porta 8000:
```bash
nc -l -p 8000
```

### No Host (Envio / Cliente)
No host, conectou-se ao IP do Target na porta 8000 enviando texto:
```bash
nc 192.168.1.10 8000
```

Texto enviado para teste:
```text
Ola mundo via Netcat!
Teste de Comunicacao entre Host e Target realizado com sucesso.
```

---

## 3. Resultado Obtido no Target

O `nc` em execução no target recebeu e exibiu com fidelidade os dados enviados pelo host:

```text
Ola mundo via Netcat!
Teste de Comunicacao entre Host e Target realizado com sucesso.
```

---

## 4. Análise dos Resultados
- **Conectividade:** Conexão TCP estabelecida instantaneamente entre Host e Guest na porta 8000.
- **Integridade:** Todo o fluxo de caracteres digitados/enviados no host chegou ao target sem truncamento ou corrupção.
- **Conclusão:** O teste 3.3 comprovou a capacidade de troca direta e bidirecional de mensagens e fluxos de dados via sockets TCP entre o sistema host e a máquina virtual.

