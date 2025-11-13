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
* Para CT2: if ((c == '\n' || c == '\r') && rx_buf_pos > 0) FOI PARA if (c == '\n' || c == '\r' && rx_buf_pos >= 0). Dessa forma quando o enter é pressionado, a mensagem é enviada e não há um número mínimo de caractéres que precisa ser atendido para a mensagem ser admitida (a não ser que ela seja negativa, o que não faz sentido algum, mas previne esse erro.
* Para CT3: #define MSG_SIZE 32 FOI PARA #define MSG_SIZE 512. Dessa forma a mensagem lida pode ter até 511 caracteres (1 no final para o terminador)

## 3.4 Evidências de Funcionamento

* [Link para o log CT1] https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/evidence/echo/ct1.txt
* [Link para o log CT2] https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/evidence/echo/ct2.txt
* [Link para o log CT3] https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/evidence/echo/ct3.txt

** Trechos ilustrativos:

* Hello! I'm your echo bot. 
Tell me something and press enter:
---- Sent utf8 encoded message: "Olá Mundo! (o tal do hello world)\r" ----
Echo: Olá Mundo! (o tal do hello world)

* Hello! I'm your echo bot. 
Tell me something and press enter:
---- Sent utf8 encoded message: "MEU DEUS JÁ É QUASE FÉRIAS, LOGO EU VOU ESTAR LIVREEEEEEEEEEEEEEEEEEEEEEEE\r" ----
Echo: MEU DEUS JÁ É QUASE FÉRIAS, LOGO EU VOU ESTAR LIVREEEEEEEEEEEEEEEEEEEEEEEE

## 3.5 Diagramas de Sequência D2

* CT1:
https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/sequence-diagrams/echo/ct1.png
shape: sequence_diagram
User
App
UART_Driver
UART_Hardware
User -> UART_Hardware: "amogus"
UART_Hardware -> UART_Driver: RX Interrupt
UART_Driver -> App: serial_cb()
App -> UART_Driver: uart_poll_out(char)
UART_Driver -> UART_Hardware: (envia dados)
UART_Hardware -> User: "Echo: amogus"

* CT2:
https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/sequence-diagrams/echo/ct2.png
shape: sequence_diagram
User
App
UART_Driver
UART_Hardware
User -> UART_Hardware: "___"
UART_Hardware -> UART_Driver: RX Interrupt
UART_Driver -> App: serial_cb()
App -> UART_Driver: uart_poll_out(char)
UART_Driver -> UART_Hardware: (envia dados)
UART_Hardware -> User: "Echo: ___"

* CT3:
https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/sequence-diagrams/echo/ct3.png
shape: sequence_diagram
User
App
UART_Driver
UART_Hardware
User -> UART_Hardware: "1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17"
UART_Hardware -> UART_Driver: RX Interrupt
UART_Driver -> App: serial_cb()
App -> UART_Driver: uart_poll_out(char)
UART_Driver -> UART_Hardware: (envia dados)
UART_Hardware -> User: "Echo: 1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17"

---

# 4. Etapa 2 – "Async" com interrupção (Transmissão/Recepção)  

## 4.1 Descrição do Funcionamento

Similar ao do UART Echo, mas o microcontrolador fica permanentemente em um estado de recebimento ou envio de dados (5s cada), com a CPU sempre ocupada por esse. Foi feito de tal forma que há possibilidade do microcontrolador se auto alimentar com informações geradas aleatoriamente ou receber informações de outra placa (relação escravo-mestre nesse caso). O código utilizado é baseado no Echo da etapa 3.

## 4.2 Casos de Teste Planejados (TDD)

### CT1 – Transmissão de pacotes a cada 5s

* Teste realizado:
* Resultado esperado: 
* Critério de Aceitação: 

### CT2 – Recepção

* Teste realizado: 
* Resultado esperado: 
* Critério de Aceitação:

### CT3 – Verificação de timing dos 5s

* Teste realizado: Cronometragem manual do tempo entre o início da recepção e da transmissão e o fim delas
* Resultado esperado: Todos os tempos registrados próximos da casa de 5000ms
* Critério de Aceitação: Todos os tempos registrados próximos da casa de 5000ms

## 4.3 Implementação

* Arquivos modificados: main.c
* Motivos/Justificativas: O código base do UART Echo não possui, em sua forma base, um período de recepção e envio de dados ou pinos para troca de informações entre duas placas, então estes tiveram de ser adicionados.
* ADENDO: como a estrutura do main branch do github já havia sido feita previamente, os códigos, evidências e diagramas de sequência foram postos nas pastas com título de "async", apesar desse projeto não ser propriamente uma implementação async.

## 4.4 Evidências de Funcionamento

AAA

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

AAA

---

# 5. Conclusões da Dupla

* O que deu certo: UART Echo funcionou de forma quase perfeita direto da caixa, sendo feitas apenas duas modificações no código para tratar de inconsistências em mensagens sem nada e mensagens muito grandes. A implementação dos períodos de recepção e transmissão e de seus respectivos pinos também foi um sucesso.
* O que foi mais desafiador: Antes de realizar uma comunicação entre duas placas com interrupção (Async com interrupção), as inúmeras tentativas para fazer o código original do Async API foram extremamente desafiadoras e não retornaram nenhum fruto. As etapas que foram realmente efetivadas (Echo e Interrupção) também tiveram seus desafios, principalmente na parte de entendimento do código e comunicação entre as placas, mas foi possível realizá-las.
