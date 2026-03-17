# 3986553-Ativ01-A
Sistema contador de pessoas com PIC16F877A utilizando sensores e LEDs 
# Contador de Pessoas com PIC16F877A

**Autor:** Alicio Rodrigues de Souza  
**RU:** 3986553  
**Curso:** Engenharia da Computação  

## Descrição do Projeto

Este projeto implementa um sistema embarcado para contagem de pessoas em um ambiente com **duas portas**, utilizando o microcontrolador **PIC16F877A**.

Cada porta possui **dois sensores ópticos com LDR**, permitindo identificar a direção de passagem da pessoa. A lógica do sistema determina se ocorreu **entrada** ou **saída** com base na ordem de ativação dos sensores.

O sistema também possui:

- **LED de status** indicando que o sistema está em funcionamento
- **LED de ocupado** indicando se há pessoas dentro do ambiente
- **LED de entrada** indicando evento de entrada
- **LED de saída** indicando evento de saída
- **LCD 16x2** exibindo:
  - total de entradas
  - total de saídas
  - quantidade de pessoas dentro do ambiente
- **botão de reset** para zerar os contadores

## Funcionalidades

- Detecta entrada pela ordem correta dos sensores
- Detecta saída pela ordem correta dos sensores
- Mantém a contagem de pessoas dentro do ambiente
- Só permite registrar saída se houver pelo menos uma pessoa dentro
- Exibe informações em um display LCD 16x2
- Indica eventos por LEDs
- Possui tempo máximo de 2 segundos entre sensores para validar a sequência

## Hardware Utilizado

- 1 microcontrolador **PIC16F877A**
- 1 placa didática compatível com **PIC-DIP40**
- 1 gravador **PICkit 3.5**
- 4 sensores com **LDR**
- 4 LEDs emissores para os sensores ópticos
- 4 resistores de **10 kΩ** para os divisores dos LDRs
- 4 LEDs de indicação:
  - status
  - ocupado
  - entrada
  - saída
- 4 resistores de **330 Ω** para os LEDs de indicação
- 1 display **LCD 16x2**
- 1 potenciômetro de **10 kΩ** para contraste do LCD
- 1 botão de reset
- Protoboard e jumpers
- Fonte de **5 V**

## Lógica de Funcionamento

### Porta 1
- **S1 → S2** = entrada
- **S2 → S1** = saída

### Porta 2
- **S3 → S4** = entrada
- **S4 → S3** = saída

A lógica utiliza uma janela de **2 segundos** entre o primeiro e o segundo sensor.  
Se esse tempo for ultrapassado, o evento é descartado.

Após uma contagem válida, a porta fica bloqueada até que os dois sensores retornem ao estado normal.

## Tabela de Conexões do PIC16F877A

### Alimentação

| Pino PIC | Nome | Ligação |
|---|---|---|
| 11 | VDD | +5 V |
| 12 | VSS | GND |
| 32 | VDD | +5 V |
| 31 | VSS | GND |

### Oscilador

| Pino PIC | Nome | Ligação |
|---|---|---|
| 13 | OSC1/CLKI | cristal 4 MHz |
| 14 | OSC2/CLKO | cristal 4 MHz |

> Se a placa didática já possuir cristal montado, não é necessário montar externamente.

### Sensores

| Pino PIC | Função |
|---|---|
| RB0 (pino 33) | SENSOR_S1 |
| RB1 (pino 34) | SENSOR_S2 |
| RB2 (pino 35) | SENSOR_S3 |
| RB3 (pino 36) | SENSOR_S4 |
| RB4 (pino 37) | BOTÃO_RESET |

### LEDs de Indicação

| Pino PIC | Função |
|---|---|
| RC1 (pino 16) | LED_STATUS |
| RC2 (pino 17) | LED_OCUPADO |
| RC3 (pino 18) | LED_ENTRADA |
| RC4 (pino 23) | LED_SAIDA |

### LCD 16x2

O LCD está ligado em **modo 8 bits**, conforme o esquemático da placa.

| Pino LCD | Função | Ligação no PIC |
|---|---|---|
| 1 | GND | GND |
| 2 | VCC | +5 V |
| 3 | VO | cursor do potenciômetro 10 kΩ |
| 4 | RS | RE0 |
| 5 | RW | RE1 |
| 6 | EN | RE2 |
| 7 | D0 | RD0 |
| 8 | D1 | RD1 |
| 9 | D2 | RD2 |
| 10 | D3 | RD3 |
| 11 | D4 | RD4 |
| 12 | D5 | RD5 |
| 13 | D6 | RD6 |
| 14 | D7 | RD7 |
| 15 | BLA | +5 V |
| 16 | BLK | GND |

## Montagem dos Sensores LDR

Cada sensor deve ser montado como divisor de tensão:


+5V ---- LDR ----+---- entrada do PIC
                 |
               10kΩ
                 |
                GND
Montagem dos LEDs

Os LEDs de indicação foram considerados em lógica ativa baixa.

Ligação recomendada:

+5V ---- resistor 330Ω ---- anodo do LED
catodo do LED ---- pino do PIC
Significado dos LEDs

LED_STATUS: pisca continuamente indicando que o sistema está funcionando
LED_OCUPADO: acende quando houver pessoas dentro
LED_ENTRADA: pisca quando uma entrada é detectada
LED_SAIDA: pisca quando uma saída é detectada

Funcionamento

botão solto → nível lógico 0

botão pressionado → nível lógico 1

Gravação do Firmware

Abrir o projeto no MPLAB X IDE

Compilar com o XC8

Gerar o arquivo .hex

Gravar no PIC16F877A utilizando o PICkit 3.5

Arquivos do Projeto

3986553-Ativ01-A.c → código-fonte

arquivo.hex → arquivo compilado para gravação

README.md → documentação do projeto

Como Testar
Teste de entrada pela porta 1

acionar S1 primeiro
depois acionar S2

resultado esperado:

incrementa total de entradas
incrementa pessoas dentro
LED de entrada pisca
LED de ocupado acende se houver pelo menos 1 pessoa

Teste de saída pela porta 1

acionar S2 primeiro
depois acionar S1
resultado esperado:

decrementa pessoas dentro
incrementa total de saídas
LED de saída pisca
Regra importante

Testes equivalentes na porta 2

S3 → S4 = entrada
S4 → S3 = saída
Software Utilizado
MPLAB X IDE v6.00
Compilador XC8

Observações Técnicas

Se houver instabilidade nos sensores, revisar alinhamento entre LED emissor e LDR
Evitar interferência de luz ambiente
Confirmar alimentação estável em 5 V

