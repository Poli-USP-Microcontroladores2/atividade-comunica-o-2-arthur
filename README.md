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

* [main] https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/src/echo/main.c

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

* Entrada:
```
"1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26.27.28.29.30.31.32.33.34.35.36.37.38.39.40.41.42.43.44.45.46.47.48.49.50.51.52.53.54.55.56.57.58.59.60.61.62.63.64.65.66.67.68.69.70.71.72.73.74.75.76.77.78.79.80.81.82.83.84.85.86.87.88.89.90.91.92.93.94.95.96.97.98.99.100"
```
* Saída esperada: "Echo:
```  1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26.27.28.29.30.31.32.33.34.35.36.37.38.39.40.41.42.43.44.45.46.47.48.49.50.51.52.53.54.55.56.57.58.59.60.61.62.63.64.65.66.67.68.69.70.71.72.73.74.75.76.77.78.79.80.81.82.83.84.85.86.87.88.89.90.91.92.93.94.95.96.97.98.99.100"
```
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
```
* Hello! I'm your echo bot. 
Tell me something and press enter:
---- Sent utf8 encoded message: "Olá Mundo! (o tal do hello world)\r" ----
Echo: Olá Mundo! (o tal do hello world)

* Hello! I'm your echo bot. 
Tell me something and press enter:
---- Sent utf8 encoded message: "MEU DEUS JÁ É QUASE FÉRIAS, LOGO EU VOU ESTAR LIVREEEEEEEEEEEEEEEEEEEEEEEE\r" ----
Echo: MEU DEUS JÁ É QUASE FÉRIAS, LOGO EU VOU ESTAR LIVREEEEEEEEEEEEEEEEEEEEEEEE
```

## 3.5 Diagramas de Sequência D2

* CT1:
* https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/sequence-diagrams/echo/ct1.png
```
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
```

* CT2:
* https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/sequence-diagrams/echo/ct2.png
```
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
```

* CT3:
* https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/sequence-diagrams/echo/ct3.png
```
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
```

---

# 4. Etapa 2 – "Async" com interrupção (Transmissão/Recepção)  

## 4.1 Descrição do Funcionamento

É similar ao do UART Echo, mas os microcontroladores ficam permanentemente em um estado de recebimento ou envio de dados (5s cada), com a CPU sempre ocupada por esses processos. Ambos os microcontroladores ficam lendo o pino PTB0, PTB1 e PTB2, esperando o pressionamento de um botão, ou ficam aguardando um sinal UART da outra placa. O botão envia os dados (0 ou 1, ligado ou desligado), fazendo com que o LED do microcontrolador, em que o botão foi pressionado, desligue e envie o estado atual do LED pela sua UART para o outro microcontrolador. Depois de, no máximo 5 segundos, o outro micro entra em estado de recebimento de dados e, por consequência, desliga o seu LED. Importante notar que o PTB0/PTB1/PTB2 possui prioridade maior do que o sinal recebido pela UART em relação ao controle do LED, isso foi feito para evitar o problema de re-alimentação cruzada descrito em um dos casos de teste. Por fim, também é possível fazer relações de cores RGB hexadecimal entre as placas. O código utilizado é baseado no Echo da etapa 3.

* [main] https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/src/async/main.c

## 4.2 Casos de Teste Planejados (TDD)

### CT1 – Transmissão de pacotes a cada 5s

* Teste realizado: Cronometragem manual do tempo entre a ativação do PTB0/PTB1/PTB2 de uma placa e a atualização do estado do LED da outra
* Resultado esperado: O tempo entre esses eventos é de, no máximo, 5 segundos, aproximadamente
* Critério de Aceitação: O resultado esperado seja atendido

### CT2 – Recepção

* Teste realizado: Verificação se o microcontrolador alterna entre estados de envio e recepção a cada 5 segundos, monitorando a mudança de estado do LED quando um sinal UART é recebido
* Resultado esperado: O LED do microcontrolador receptor deve alterar seu estado (ligar/desligar) conforme o dado recebido via UART
* Critério de Aceitação: O resultado esperado seja atendido

### CT3 – Verificação de timing dos 5s

* Teste realizado: Medição do tempo que cada microcontrolador permanece em estado de transmissão antes de alternar para recepção
* Resultado esperado: Cada período de transmissão/recepção deve durar exatamente 5 segundos
* Critério de Aceitação: O resultado esperado seja atendido

### CT4 – Re-alimentação cruzada pelas UARTS

* Teste realizado: Acionar o botão de uma das placas para ligar, desligar e ligar novamente dentro do período de transmissão da outra placa
* Resultado esperado: A outra placa, ao chegar no seu período de recepção, não desligará o seu LED
* Critério de Aceitação: O resultado esperado seja atendido

## 4.3 Implementação

* Arquivos modificados: main.c
* Motivos/Justificativas: O código base do UART Echo não possui, em sua forma base, um período de recepção, envio de dados ou pinos para troca de informações entre duas placas e o pino LED, então estes tiveram de ser adicionados.
* ADENDO: como a estrutura do main branch do github já havia sido feita previamente, os códigos, evidências e diagramas de sequência foram postos nas pastas com título de "async", apesar desse projeto não ser propriamente uma implementação async.

## 4.4 Evidências de Funcionamento

* [Vídeo que comprova CT1-4] https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/evidence/async/WhatsApp%20Video%202025-11-13%20at%2016.26.00(1).mp4

## 4.5 Diagramas de Sequência D2

* CT1-4
* https://github.com/Poli-USP-Microcontroladores2/atividade-comunica-o-2-arthur/blob/main/docs/sequence-diagrams/async/d2(3).png

```
# Define a forma como um diagrama de sequência
shape: sequence_diagram

# Declara os "atores" do diagrama
MCU_A
MCU_B

# --- Fluxo: MCU_A envia seu estado para MCU_B ---

# A thread TX em A acorda (a cada 5s)
MCU_A -> MCU_A: "TX_Thread: Acorda (timer 5s)"

# A thread lê os pinos de entrada locais
MCU_A -> MCU_A: "TX_Thread: Lê inputs locais (ex: valor '5')"

# A thread envia o caractere '5' pela UART
MCU_A -> MCU_B: "TX_Thread: uart_fifo_fill('5')"

# MCU_B recebe uma interrupção de hardware
MCU_B -> MCU_B: "Hardware: RX Interrupt"

# O callback (ISR) é executado
MCU_B -> MCU_B: "ISR: chama uart_rx_cb()"

# O callback atualiza a variável global
MCU_B -> MCU_B: "ISR: atomic_set(&rx_state, 5)"

# O loop principal em B (que roda a cada 10ms) eventualmente lê o novo estado
MCU_B -> MCU_B: "Main_Loop: Lê rx_state (obtém 5)"

# O loop principal calcula o estado final do LED e o atualiza
MCU_B -> MCU_B: "Main_Loop: Atualiza LEDs (prioridade local)"

# --- Fluxo: MCU_B envia seu estado para MCU_A (acontece em paralelo) ---

# A thread TX em B também acorda
MCU_B -> MCU_B: "TX_Thread: Acorda (timer 5s)"

# A thread B lê seus próprios pinos
MCU_B -> MCU_B: "TX_Thread: Lê inputs locais (ex: valor '3')"

# A thread B envia seu estado
MCU_B -> MCU_A: "TX_Thread: uart_fifo_fill('3')"

# MCU_A recebe a interrupção
MCU_A -> MCU_A: "Hardware: RX Interrupt"

# O callback em A é executado
MCU_A -> MCU_A: "ISR: chama uart_rx_cb()"

# O estado recebido em A é atualizado
MCU_A -> MCU_A: "ISR: atomic_set(&rx_state, 3)"

# O loop principal em A lê o estado de B
MCU_A -> MCU_A: "Main_Loop: Lê rx_state (obtém 3)"

# A atualiza seus LEDs
MCU_A -> MCU_A: "Main_Loop: Atualiza LEDs (prioridade local)"
```

---

# 5. Conclusões da Dupla

* O que deu certo: UART Echo funcionou de forma quase perfeita direto da caixa, sendo feitas apenas duas modificações no código para tratar de inconsistências em mensagens sem nada e mensagens muito grandes. A implementação dos períodos de recepção e transmissão e de seus respectivos pinos e do pino LED também foi um sucesso.
* O que foi mais desafiador: Antes de realizar uma comunicação entre duas placas com interrupção (Async com interrupção), as inúmeras tentativas para fazer o código original do Async API foram extremamente desafiadoras e não retornaram nenhum fruto. As etapas que foram realmente efetivadas (Echo e Interrupção) também tiveram seus desafios, principalmente na parte de entendimento do código e comunicação entre as placas, mas foi possível realizá-las.
