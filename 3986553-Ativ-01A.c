// ============================================================================
// ARQUIVO: 3986553-Ativ01-A.c
// ALUNO: Alicio Rodrigues de Souza
// RU: 3986553
// CURSO: Engenharia da Computacao
// DISCIPLINA: Sistemas Embarcados
// MICROCONTROLADOR: PIC16F877A
// COMPILADOR: MPLAB XC8
// PIC-DIP40 Versao 2.0
// OSCILADOR: 4 MHz
// DESCRICAO:
// Contador de pessoas com duas portas usando sensores LDR.
//
// LOGICA UTILIZADA:
// PORTA 1:
//   ENTRADA -> S1 ativa primeiro e depois S2 ativa dentro de 2 segundos
//   SAIDA   -> S2 ativa primeiro e depois S1 ativa dentro de 2 segundos
//
// PORTA 2:
//   ENTRADA -> S3 ativa primeiro e depois S4 ativa dentro de 2 segundos
//   SAIDA   -> S4 ativa primeiro e depois S3 ativa dentro de 2 segundos
//
// OBSERVACOES:
// - Os sensores ficam em nivel alto quando o feixe e cortado.
// - Os LEDs estao em logica ativa baixa.
// - A contagem e confirmada quando o segundo sensor ativa.
// - Depois da contagem, a porta fica bloqueada ate os dois sensores voltarem a zero.
// - O piscar dos LEDs de evento nao bloqueia o programa.
// ============================================================================


// -----------------------------------------------------------------------------
// Biblioteca principal
// -----------------------------------------------------------------------------
#include <xc.h>                                                  // Inclui definicoes do PIC16F877A


// -----------------------------------------------------------------------------
// Configuracao dos fusíveis
// -----------------------------------------------------------------------------
#pragma config FOSC = XT                                         // Usa cristal externo de 4 MHz
#pragma config WDTE = OFF                                        // Desliga watchdog
#pragma config PWRTE = ON                                        // Liga power-up timer
#pragma config BOREN = ON                                        // Liga brown-out reset
#pragma config LVP = OFF                                         // Desliga programacao em baixa tensao
#pragma config CPD = OFF                                         // Desliga protecao da EEPROM
#pragma config WRT = OFF                                         // Desliga protecao de escrita
#pragma config CP = OFF                                          // Desliga protecao de codigo


// -----------------------------------------------------------------------------
// Frequencia do cristal
// -----------------------------------------------------------------------------
#define _XTAL_FREQ 4000000                                       // Define cristal de 4 MHz


// -----------------------------------------------------------------------------
// Entradas
// -----------------------------------------------------------------------------
#define S1_6553 RB0                                              // Sensor 1 da porta 1
#define S2_6553 RB1                                              // Sensor 2 da porta 1
#define S3_6553 RB2                                              // Sensor 1 da porta 2
#define S4_6553 RB3                                              // Sensor 2 da porta 2
#define RESET_6553 RB4                                           // Botao de reset


// -----------------------------------------------------------------------------
// Saidas
// -----------------------------------------------------------------------------
#define LED_STATUS_6553 RD0                                      // LED de funcionamento
#define LED_ENTRADA_6553 RD1                                     // LED de entrada
#define LED_SAIDA_6553 RD2                                       // LED de saida
#define LED_OCUPADO_6553 RD3                                     // LED de ocupado


// -----------------------------------------------------------------------------
// Constantes de tempo
// -----------------------------------------------------------------------------
#define LOOP_MS_6553 2                                           // Loop rapido de 2 ms
#define TIMEOUT_CICLOS_6553 1000                                 // 1000 x 2 ms = 2 segundos
#define STATUS_TOGGLE_CICLOS_6553 125                            // 125 x 2 ms = 250 ms
#define PULSO_LED_CICLOS_6553 60                                 // 60 x 2 ms = 120 ms
#define DEBOUNCE_RESET_CICLOS_6553 15                            // 15 x 2 ms = 30 ms


// -----------------------------------------------------------------------------
// Estados da porta
// -----------------------------------------------------------------------------
#define PORTA_LIVRE_6553 0                                       // Porta livre para iniciar nova leitura
#define PORTA_POSSIVEL_ENTRADA_6553 1                            // Primeiro sensor de entrada foi ativado
#define PORTA_POSSIVEL_SAIDA_6553 2                              // Primeiro sensor de saida foi ativado
#define PORTA_BLOQUEADA_6553 3                                   // Evento ja contado, aguardando liberar os dois sensores


// -----------------------------------------------------------------------------
// Variaveis globais
// -----------------------------------------------------------------------------
unsigned int CONTADOR_PESSOAS_6553 = 0;                          // Quantidade de pessoas no ambiente

unsigned char ESTADO_P1_6553 = PORTA_LIVRE_6553;                 // Estado atual da porta 1
unsigned char ESTADO_P2_6553 = PORTA_LIVRE_6553;                 // Estado atual da porta 2

unsigned int TIMER_P1_6553 = 0;                                  // Temporizador da porta 1
unsigned int TIMER_P2_6553 = 0;                                  // Temporizador da porta 2
unsigned int TIMER_STATUS_6553 = 0;                              // Temporizador do LED de status

unsigned int TIMER_LED_ENTRADA_6553 = 0;                         // Temporizador do pulso do LED de entrada
unsigned int TIMER_LED_SAIDA_6553 = 0;                           // Temporizador do pulso do LED de saida
unsigned int TIMER_RESET_6553 = 0;                               // Temporizador de debounce do reset

unsigned char RESET_TRAVA_6553 = 0;                              // Evita repeticao continua do reset


// -----------------------------------------------------------------------------
// Prototipos
// -----------------------------------------------------------------------------
void inicializarSistema6553(void);                               // Inicializa hardware e variaveis
void atualizarLedStatus6553(void);                               // Pisca LED de funcionamento
void atualizarLedOcupado6553(void);                              // Atualiza LED de ocupado
void iniciarPulsoEntrada6553(void);                              // Inicia pulso do LED de entrada
void iniciarPulsoSaida6553(void);                                // Inicia pulso do LED de saida
void atualizarPulsosEvento6553(void);                            // Atualiza pulso dos LEDs de evento sem bloquear
void registrarEntrada6553(void);                                 // Soma 1 no contador
void registrarSaida6553(void);                                   // Subtrai 1 no contador
void tratarReset6553(void);                                      // Trata o botao de reset
void tratarPorta1_6553(void);                                    // Processa a porta 1
void tratarPorta2_6553(void);                                    // Processa a porta 2


// -----------------------------------------------------------------------------
// Inicializacao do sistema
// -----------------------------------------------------------------------------
void inicializarSistema6553(void)
{
    ADCON1 = 0x06;                                               // Configura entradas analogicas como digitais
    OPTION_REGbits.nRBPU = 1;                                    // Desliga pull-up interno da PORTB
    TRISB = 0b00011111;                                          // RB0 a RB4 como entrada
    TRISD = 0b00000000;                                          // PORTD como saida

    PORTB = 0x00;                                                // Inicializa PORTB
    PORTD = 0xFF;                                                // Inicializa LEDs apagados em logica ativa baixa

    LED_STATUS_6553 = 1;                                         // LED status inicia apagado
    LED_ENTRADA_6553 = 1;                                        // LED entrada inicia apagado
    LED_SAIDA_6553 = 1;                                          // LED saida inicia apagado
    LED_OCUPADO_6553 = 1;                                        // LED ocupado inicia apagado

    CONTADOR_PESSOAS_6553 = 0;                                   // Zera contador

    ESTADO_P1_6553 = PORTA_LIVRE_6553;                           // Porta 1 inicia livre
    ESTADO_P2_6553 = PORTA_LIVRE_6553;                           // Porta 2 inicia livre

    TIMER_P1_6553 = 0;                                           // Zera timer porta 1
    TIMER_P2_6553 = 0;                                           // Zera timer porta 2
    TIMER_STATUS_6553 = 0;                                       // Zera timer status
    TIMER_LED_ENTRADA_6553 = 0;                                  // Zera timer LED entrada
    TIMER_LED_SAIDA_6553 = 0;                                    // Zera timer LED saida
    TIMER_RESET_6553 = 0;                                        // Zera timer reset

    RESET_TRAVA_6553 = 0;                                        // Libera reset

    __delay_ms(300);                                             // Aguarda estabilizacao inicial
}


// -----------------------------------------------------------------------------
// Atualiza LED de status
// -----------------------------------------------------------------------------
void atualizarLedStatus6553(void)
{
    TIMER_STATUS_6553++;                                         // Incrementa temporizador do LED de status

    if (TIMER_STATUS_6553 >= STATUS_TOGGLE_CICLOS_6553)          // Verifica se chegou a hora de alternar
    {
        LED_STATUS_6553 = !LED_STATUS_6553;                      // Inverte LED de status
        TIMER_STATUS_6553 = 0;                                   // Reinicia temporizador
    }
}


// -----------------------------------------------------------------------------
// Atualiza LED de ocupado
// -----------------------------------------------------------------------------
void atualizarLedOcupado6553(void)
{
    if (CONTADOR_PESSOAS_6553 > 0)                               // Se houver pessoas no ambiente
    {
        LED_OCUPADO_6553 = 0;                                    // Liga LED ocupado
    }
    else                                                         // Se nao houver pessoas
    {
        LED_OCUPADO_6553 = 1;                                    // Desliga LED ocupado
    }
}


// -----------------------------------------------------------------------------
// Inicia pulso do LED de entrada
// -----------------------------------------------------------------------------
void iniciarPulsoEntrada6553(void)
{
    LED_ENTRADA_6553 = 0;                                        // Liga LED de entrada
    TIMER_LED_ENTRADA_6553 = PULSO_LED_CICLOS_6553;              // Carrega tempo do pulso
}


// -----------------------------------------------------------------------------
// Inicia pulso do LED de saida
// -----------------------------------------------------------------------------
void iniciarPulsoSaida6553(void)
{
    LED_SAIDA_6553 = 0;                                          // Liga LED de saida
    TIMER_LED_SAIDA_6553 = PULSO_LED_CICLOS_6553;                // Carrega tempo do pulso
}


// -----------------------------------------------------------------------------
// Atualiza pulsos dos LEDs de evento sem travar a leitura
// -----------------------------------------------------------------------------
void atualizarPulsosEvento6553(void)
{
    if (TIMER_LED_ENTRADA_6553 > 0)                              // Verifica se pulso de entrada esta ativo
    {
        TIMER_LED_ENTRADA_6553--;                                // Decrementa tempo restante

        if (TIMER_LED_ENTRADA_6553 == 0)                         // Verifica fim do pulso
        {
            LED_ENTRADA_6553 = 1;                                // Desliga LED de entrada
        }
    }

    if (TIMER_LED_SAIDA_6553 > 0)                                // Verifica se pulso de saida esta ativo
    {
        TIMER_LED_SAIDA_6553--;                                  // Decrementa tempo restante

        if (TIMER_LED_SAIDA_6553 == 0)                           // Verifica fim do pulso
        {
            LED_SAIDA_6553 = 1;                                  // Desliga LED de saida
        }
    }
}


// -----------------------------------------------------------------------------
// Registra entrada valida
// -----------------------------------------------------------------------------
void registrarEntrada6553(void)
{
    CONTADOR_PESSOAS_6553++;                                     // Incrementa contador
    iniciarPulsoEntrada6553();                                   // Inicia pulso do LED verde
    atualizarLedOcupado6553();                                   // Atualiza LED ocupado
}


// -----------------------------------------------------------------------------
// Registra saida valida
// -----------------------------------------------------------------------------
void registrarSaida6553(void)
{
    if (CONTADOR_PESSOAS_6553 > 0)                               // Evita contador negativo
    {
        CONTADOR_PESSOAS_6553--;                                 // Decrementa contador
    }

    iniciarPulsoSaida6553();                                     // Inicia pulso do LED amarelo
    atualizarLedOcupado6553();                                   // Atualiza LED ocupado
}


// -----------------------------------------------------------------------------
// Trata reset com debounce simples
// -----------------------------------------------------------------------------
void tratarReset6553(void)
{
    if (RESET_6553)                                              // Verifica se botao de reset esta pressionado
    {
        if (TIMER_RESET_6553 < DEBOUNCE_RESET_CICLOS_6553)       // Incrementa debounce enquanto pressionado
        {
            TIMER_RESET_6553++;
        }
        else                                                     // Quando tempo de debounce e atingido
        {
            if (RESET_TRAVA_6553 == 0)                           // Garante um reset por pressionamento
            {
                CONTADOR_PESSOAS_6553 = 0;                       // Zera contador
                ESTADO_P1_6553 = PORTA_LIVRE_6553;               // Reinicia porta 1
                ESTADO_P2_6553 = PORTA_LIVRE_6553;               // Reinicia porta 2
                TIMER_P1_6553 = 0;                               // Zera timer porta 1
                TIMER_P2_6553 = 0;                               // Zera timer porta 2
                atualizarLedOcupado6553();                       // Atualiza LED ocupado
                RESET_TRAVA_6553 = 1;                            // Trava reset ate soltar botao
            }
        }
    }
    else                                                         // Quando botao nao esta pressionado
    {
        TIMER_RESET_6553 = 0;                                    // Zera debounce
        RESET_TRAVA_6553 = 0;                                    // Libera novo reset futuro
    }
}


// -----------------------------------------------------------------------------
// Processa porta 1 por ordem de ativacao
// -----------------------------------------------------------------------------
void tratarPorta1_6553(void)
{
    if (ESTADO_P1_6553 == PORTA_LIVRE_6553)                      // Se porta 1 estiver livre
    {
        if (S1_6553 && !S2_6553)                                 // S1 ativou primeiro
        {
            ESTADO_P1_6553 = PORTA_POSSIVEL_ENTRADA_6553;        // Marca possivel entrada
            TIMER_P1_6553 = 0;                                   // Reinicia timer
        }
        else if (S2_6553 && !S1_6553)                            // S2 ativou primeiro
        {
            ESTADO_P1_6553 = PORTA_POSSIVEL_SAIDA_6553;          // Marca possivel saida
            TIMER_P1_6553 = 0;                                   // Reinicia timer
        }
    }
    else if (ESTADO_P1_6553 == PORTA_POSSIVEL_ENTRADA_6553)      // Se houver possivel entrada
    {
        TIMER_P1_6553++;                                         // Incrementa tempo da sequencia

        if (S2_6553)                                             // Se segundo sensor ativou dentro da janela
        {
            registrarEntrada6553();                              // Confirma entrada
            ESTADO_P1_6553 = PORTA_BLOQUEADA_6553;               // Bloqueia ate ambos zerarem
            TIMER_P1_6553 = 0;                                   // Zera timer
        }
        else if (TIMER_P1_6553 > TIMEOUT_CICLOS_6553)            // Se passou de 2 segundos
        {
            ESTADO_P1_6553 = PORTA_LIVRE_6553;                   // Cancela sequencia
            TIMER_P1_6553 = 0;                                   // Zera timer
        }
    }
    else if (ESTADO_P1_6553 == PORTA_POSSIVEL_SAIDA_6553)        // Se houver possivel saida
    {
        TIMER_P1_6553++;                                         // Incrementa tempo da sequencia

        if (S1_6553)                                             // Se segundo sensor ativou dentro da janela
        {
            registrarSaida6553();                                // Confirma saida
            ESTADO_P1_6553 = PORTA_BLOQUEADA_6553;               // Bloqueia ate ambos zerarem
            TIMER_P1_6553 = 0;                                   // Zera timer
        }
        else if (TIMER_P1_6553 > TIMEOUT_CICLOS_6553)            // Se passou de 2 segundos
        {
            ESTADO_P1_6553 = PORTA_LIVRE_6553;                   // Cancela sequencia
            TIMER_P1_6553 = 0;                                   // Zera timer
        }
    }
    else if (ESTADO_P1_6553 == PORTA_BLOQUEADA_6553)             // Estado bloqueado ate sensores liberarem
    {
        if ((!S1_6553) && (!S2_6553))                            // Quando os dois sensores estiverem livres
        {
            ESTADO_P1_6553 = PORTA_LIVRE_6553;                   // Libera nova contagem
        }
    }
}


// -----------------------------------------------------------------------------
// Processa porta 2 por ordem de ativacao
// -----------------------------------------------------------------------------
void tratarPorta2_6553(void)
{
    if (ESTADO_P2_6553 == PORTA_LIVRE_6553)                      // Se porta 2 estiver livre
    {
        if (S3_6553 && !S4_6553)                                 // S3 ativou primeiro
        {
            ESTADO_P2_6553 = PORTA_POSSIVEL_ENTRADA_6553;        // Marca possivel entrada
            TIMER_P2_6553 = 0;                                   // Reinicia timer
        }
        else if (S4_6553 && !S3_6553)                            // S4 ativou primeiro
        {
            ESTADO_P2_6553 = PORTA_POSSIVEL_SAIDA_6553;          // Marca possivel saida
            TIMER_P2_6553 = 0;                                   // Reinicia timer
        }
    }
    else if (ESTADO_P2_6553 == PORTA_POSSIVEL_ENTRADA_6553)      // Se houver possivel entrada
    {
        TIMER_P2_6553++;                                         // Incrementa tempo da sequencia

        if (S4_6553)                                             // Se segundo sensor ativou
        {
            registrarEntrada6553();                              // Confirma entrada
            ESTADO_P2_6553 = PORTA_BLOQUEADA_6553;               // Bloqueia ate ambos zerarem
            TIMER_P2_6553 = 0;                                   // Zera timer
        }
        else if (TIMER_P2_6553 > TIMEOUT_CICLOS_6553)            // Timeout
        {
            ESTADO_P2_6553 = PORTA_LIVRE_6553;                   // Cancela sequencia
            TIMER_P2_6553 = 0;                                   // Zera timer
        }
    }
    else if (ESTADO_P2_6553 == PORTA_POSSIVEL_SAIDA_6553)        // Se houver possivel saida
    {
        TIMER_P2_6553++;                                         // Incrementa tempo da sequencia

        if (S3_6553)                                             // Se segundo sensor ativou
        {
            registrarSaida6553();                                // Confirma saida
            ESTADO_P2_6553 = PORTA_BLOQUEADA_6553;               // Bloqueia ate ambos zerarem
            TIMER_P2_6553 = 0;                                   // Zera timer
        }
        else if (TIMER_P2_6553 > TIMEOUT_CICLOS_6553)            // Timeout
        {
            ESTADO_P2_6553 = PORTA_LIVRE_6553;                   // Cancela sequencia
            TIMER_P2_6553 = 0;                                   // Zera timer
        }
    }
    else if (ESTADO_P2_6553 == PORTA_BLOQUEADA_6553)             // Estado bloqueado
    {
        if ((!S3_6553) && (!S4_6553))                            // Quando os dois sensores estiverem livres
        {
            ESTADO_P2_6553 = PORTA_LIVRE_6553;                   // Libera nova contagem
        }
    }
}


// -----------------------------------------------------------------------------
// Programa principal
// -----------------------------------------------------------------------------
void main(void)
{
    inicializarSistema6553();                                    // Inicializa hardware e variaveis

    while (1)                                                    // Loop infinito principal
    {
        atualizarLedStatus6553();                                // Atualiza LED de funcionamento
        atualizarPulsosEvento6553();                             // Atualiza pulso de LEDs sem bloquear
        tratarReset6553();                                       // Trata botao de reset
        tratarPorta1_6553();                                     // Trata logica da porta 1
        tratarPorta2_6553();                                     // Trata logica da porta 2
        atualizarLedOcupado6553();                               // Atualiza LED de ocupado

        __delay_ms(LOOP_MS_6553);                                // Mantem loop rapido
    }
}