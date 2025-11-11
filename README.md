# PSI-Microcontroladores2-Aula10
Atividade: Comunicação UART

# Projeto UART – Atividade em Duplas (Echo Bot + Async API)

## 1. Informações Gerais

* Dupla:

  * Integrante 1: Arthur Junqueira C B - 16855560
  * Integrante 2: Arthur Londero - 16855595

* Objetivo: implementar, testar e documentar aplicações de comunicação UART baseadas nos exemplos oficiais “echo_bot” e “async_api”, utilizando desenvolvimento orientado a testes, diagramas de sequência D2 e registro de evidências.

---

# 2. Estrutura Esperada do Repositório

```
README.md
src/

docs/
  evidence/
  sequence-diagrams/

```

---

# 3. Etapa 1 – Echo Bot (UART Polling/Interrupt)

## 3.1 Descrição do Funcionamento

O comportamento esperado do código oficial do site do Zephyr é o seguinte: Usuário digita → Pino RX → rx_buf (preenchido por serial callback) → uart_msgq → main loop → print_uart() → Pino TX (uart_poll_out) → Usuário vê echo. Isso é diferente do que foi demonstrado em aula, visto que, nesse caso, há callback quando caracteres chegam pela UART (recepção RX), enquanto no diagrama do professor o callback ocorre quando a transmissão TX é completada. No código oficial, a transmissão usa polling bloqueante, não interrupções.

## 3.2 Casos de Teste Planejados (TDD)

### CT1 – Eco básico

* Entrada: "amogus"
* Saída esperada: "Echo: amogus"
* Critério de Aceitação: Se o echo recebido for igual à mensagem digitada

### CT2 – Linha vazia

* Entrada: ""
* Saída esperada: "Echo:" ou "Echo: "
* Critério de Aceitação: Se o echo recebido é vazio e enviado imediatamente após a minha mensagem vazia 

### CT3 – Linha longa

* Entrada: "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26.27.28.29.30.31.32.33.34.35.36.37.38.39.40.41.42.43.44.45.46.47.48.49.50.51.52.53.54.55.56.57.58.59.60.61.62.63.64.65.66.67.68.69.70.71.72.73.74.75.76.77.78.79.80.81.82.83.84.85.86.87.88.89.90.91.92.93.94.95.96.97.98.99.100"
* Saída esperada: "Echo: 1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26.27.28.29.30.31.32.33.34.35.36.37.38.39.40.41.42.43.44.45.46.47.48.49.50.51.52.53.54.55.56.57.58.59.60.61.62.63.64.65.66.67.68.69.70.71.72.73.74.75.76.77.78.79.80.81.82.83.84.85.86.87.88.89.90.91.92.93.94.95.96.97.98.99.100"
* Critério de Aceitação: Se o echo recebido for igual à mensagem digitada e enviada em apenas uma mensagem sem ser cortada
(Adicionar mais casos se necessário.)

## 3.3 Implementação

* Arquivo(s) modificados: main.c
* Justificativa das alterações:
* > Para CT2: if ((c == '\n' || c == '\r') && rx_buf_pos > 0) FOI PARA if (c == '\n' || c == '\r'). Dessa forma quando o enter é pressionado, a mensagem é enviada e não há um número mínimo de caractéres que precisa ser atendido para a mensagem ser admitida.
  > Para CT3: #define MSG_SIZE 32 FOI PARA #define MSG_SIZE 512. Dessa forma a mensagem lida pode ter até 511 caracteres (1 no final para o terminador)

## 3.4 Evidências de Funcionamento

Salvar evidências em `docs/evidence/echo_bot/`.

Exemplo de referência no README:

```
[Link para o log CT1](docs/evidence/echo_bot/ct1_output.txt)
```

Adicionar aqui pequenos trechos ilustrativos:

```
Hello! I'm your echo bot. Tell me something and press enter:
Echo: Hello World!
```

## 3.5 Diagramas de Sequência D2

Vide material de apoio: https://d2lang.com/tour/sequence-diagrams/

Adicionar arquivos (diagrama completo e o código-base para geração do diagrama) em `docs/sequence-diagrams/`.

---

# 4. Etapa 2 – Async API (Transmissão/Recepção Assíncrona)

## 4.1 Descrição do Funcionamento

Descrever o comportamento esperado de forma textual, especialmente com a alternância TX/RX.
Link usado como referência:
[https://docs.zephyrproject.org/latest/samples/drivers/uart/async_api/README.html](https://docs.zephyrproject.org/latest/samples/drivers/uart/async_api/README.html)

## 4.2 Casos de Teste Planejados (TDD)

### CT1 – Transmissão de pacotes a cada 5s

### CT2 – Recepção

### CT3 – Verificação de timing dos 5s

(Adicionar mais casos se necessário.)

## 4.3 Implementação

* Arquivos modificados:
* Motivos/Justificativas:

## 4.4 Evidências de Funcionamento

Salvar em `docs/evidence/async_api/`.

Exemplo:

```
Loop 0:
Sending 3 packets (packet size: 5)
Packet: 0
Packet: 1
Packet: 2
```

Ou:

```
RX is now enabled
UART callback: RX_RDY
Data (HEX): 48 65 6C 6C 6F
Data (ASCII): Hello
```

## 4.5 Diagramas de Sequência D2

Vide material de referência: https://d2lang.com/tour/sequence-diagrams/

Adicionar arquivos (diagrama completo e o código-base para geração do diagrama) em `docs/sequence-diagrams/`.

---

# 5. Conclusões da Dupla

* O que deu certo:
* O que foi mais desafiador:
