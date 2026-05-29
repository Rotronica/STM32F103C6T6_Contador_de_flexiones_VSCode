#include "Display7seg.h"
#include "board_init.h"
#include "time_ticks.h"
#include "frames.h"
#include <string.h>

static uint32_t tick_save = 0;
static uint8_t display_buffer[NUM_DISPLAY];
static uint8_t display_state = 0;
static uint8_t numero_actual = 0;

static const uint8_t tabla_segmentos[] = {
    // Números 0-9
    0x3F,
    0x06,
    0x5B,
    0x4F,
    0x66,
    0x6D,
    0x7D,
    0x07,
    0x7F,
    0x6F,
    // Letras A-Z
    0x77, // A
    0x7C, // B
    0x39, // C
    0x5E, // D
    0x79, // E
    0x71, // F
    0x3D, // G
    0x76, // H
    0x30, // I
    0x1E, // J
    0x76, // K
    0x38, // L
    0x37, // M
    0x54, // N
    0x5C, // O
    0x73, // P
    0x67, // Q
    0x50, // R
    0x6D, // S
    0x78, // T
    0x1C, // U
    0x3E, // V
    0x2A, // W
    0x76, // X
    0x6E, // Y
    0x5B, // Z

    // Apagado
    0x00,
    // Símbolos especiales
    0x40, //(-)guion
};
typedef enum
{
    _A = 10,
    _B = 11,
    _C = 12,
    _D,
    _E,
    _F,
    _G,
    _H,
    _I,
    _J,
    _K,
    _L,
    _M,
    _N,
    _O,
    _P,
    _Q,
    _R,
    _S,
    _T,
    _U,
    _V,
    _W,
    _X,
    _Y,
    _Z,
    _VACIO,
    _GUION,
} abc_t;
// Prototipo de funcion
void escribir_segmento(uint8_t valor);

void Display7seg_init(void)
{
    // Apagar todos los displays
    HAL_GPIO_WritePin(GPIOA, DISPLAY_1 | DISPLAY_2 | DISPLAY_3, GPIO_PIN_RESET);
    escribir_segmento(0x00);
    // Limpiar buffer
    for (int i = 0; i < NUM_DISPLAY; i++)
    {
        display_buffer[i] = 0;
    }
    display_state = 0;
    numero_actual = 0;
}
void escribir_segmento(uint8_t valor)
{
    // PRIMERO: Apagar TODOS los segmentos (PA1 a PA7)
    HAL_GPIO_WritePin(GPIOA, PIN_A1 | PIN_A2 | PIN_A3 | PIN_A4 | PIN_A5 | PIN_A6 | PIN_A7, GPIO_PIN_RESET);
    if (valor & 0x01)
        HAL_GPIO_WritePin(GPIOA, PIN_A1, GPIO_PIN_SET); // A
    if (valor & 0x02)
        HAL_GPIO_WritePin(GPIOA, PIN_A2, GPIO_PIN_SET); // B
    if (valor & 0x04)
        HAL_GPIO_WritePin(GPIOA, PIN_A3, GPIO_PIN_SET); // C
    if (valor & 0x08)
        HAL_GPIO_WritePin(GPIOA, PIN_A4, GPIO_PIN_SET); // D
    if (valor & 0x10)
        HAL_GPIO_WritePin(GPIOA, PIN_A5, GPIO_PIN_SET); // E
    if (valor & 0x20)
        HAL_GPIO_WritePin(GPIOA, PIN_A6, GPIO_PIN_SET); // F
    if (valor & 0x40)
        HAL_GPIO_WritePin(GPIOA, PIN_A7, GPIO_PIN_SET); // G
}
void Display7seg_refresh(void)
{
    if (tick_espera(&tick_save, TIME_MUX))
    {
        // Apagar todos los displays
        HAL_GPIO_WritePin(GPIOA, DISPLAY_1 | DISPLAY_2 | DISPLAY_3, GPIO_PIN_RESET);
        // Mostrar por display
        escribir_segmento(display_buffer[display_state]);
        // Activar el display correspondiente
        switch (display_state)
        {
        case 0:
            HAL_GPIO_WritePin(GPIOA, DISPLAY_1, GPIO_PIN_SET);
            break;
        case 1:
            HAL_GPIO_WritePin(GPIOA, DISPLAY_2, GPIO_PIN_SET);
            break;
        case 2:
            HAL_GPIO_WritePin(GPIOA, DISPLAY_3, GPIO_PIN_SET);
            break;
        default:
            break;
        }
        display_state = (display_state + 1) % NUM_DISPLAY;
    }
}
// Mostrar un número en el display (actualiza el buffer)
void Display7seg_show_number(uint16_t numero)
{
    if (numero > 999)
        numero = 999;
    numero_actual = numero;

    // Limpiar buffer (opcional, para evitar residuos)
    display_buffer[0] = 0x00;
    display_buffer[1] = 0x00;
    display_buffer[2] = 0x00;

    if (numero >= 100)
    {
        // 3 dígitos: centenas, decenas, unidades
        display_buffer[0] = tabla_segmentos[numero % 10];        // Unidades (más a la derecha)
        display_buffer[1] = tabla_segmentos[(numero / 10) % 10]; // Decenas
        display_buffer[2] = tabla_segmentos[numero / 100];       // Centenas (más a la izquierda)
    }
    else if (numero >= 10)
    {
        // 2 dígitos: mostrar con un cero a la izquierda o apagado?
        display_buffer[0] = tabla_segmentos[numero % 10]; // Unidades
        display_buffer[1] = tabla_segmentos[numero / 10]; // Decenas
        display_buffer[2] = 0x00;                         // Apagar centenas
    }
    else
    {
        // 1 dígito
        display_buffer[0] = tabla_segmentos[numero]; // Unidades
        display_buffer[1] = 0x00;                    // Apagar decenas
        display_buffer[2] = 0x00;                    // Apagar centenas
    }
}
void Display7seg_show_text(const char *texto)
{
    // Limpiar buffer
    display_buffer[0] = 0x00;
    display_buffer[1] = 0x00;
    display_buffer[2] = 0x00;

    if (strcmp(texto, "inicio") == 0)
    {
        display_buffer[2] = tabla_segmentos[_GUION];
        display_buffer[1] = tabla_segmentos[_GUION];
        display_buffer[0] = tabla_segmentos[_GUION];
    }
    else if (strcmp(texto, "obj") == 0 || strcmp(texto, "objetivo") == 0)
    {
        display_buffer[2] = tabla_segmentos[_O];
        display_buffer[1] = tabla_segmentos[_B];
        display_buffer[0] = tabla_segmentos[_J];
    }
    else if (strcmp(texto, "umb") == 0 || strcmp(texto, "umbral") == 0)
    {
        display_buffer[2] = tabla_segmentos[_U];
        display_buffer[1] = tabla_segmentos[_M];
        display_buffer[0] = tabla_segmentos[_B];
    }
    else if (strcmp(texto, "arriba") == 0)
    {
        display_buffer[2] = tabla_segmentos[_VACIO];
        display_buffer[1] = tabla_segmentos[_U];
        display_buffer[0] = tabla_segmentos[_P];
    }
    else if (strcmp(texto, "abajo") == 0)
    {
        display_buffer[2] = tabla_segmentos[_D];
        display_buffer[1] = tabla_segmentos[_U];
        display_buffer[0] = tabla_segmentos[_N];
    }
    else if (strcmp(texto, "distancia") == 0)
    {
        display_buffer[2] = tabla_segmentos[_D];
        display_buffer[1] = tabla_segmentos[_S];
        display_buffer[0] = tabla_segmentos[_T];
    }
    else if (strcmp(texto, "set") == 0)
    {
        display_buffer[2] = tabla_segmentos[_S];
        display_buffer[1] = tabla_segmentos[_E];
        display_buffer[0] = tabla_segmentos[_T];
    }
    else if (strcmp(texto, "config") == 0)
    {
        display_buffer[2] = tabla_segmentos[_C];
        display_buffer[1] = tabla_segmentos[_F];
        display_buffer[0] = tabla_segmentos[_G];
    }
    else if (strcmp(texto, "bateria") == 0)
    {
        display_buffer[2] = tabla_segmentos[_B];
        display_buffer[1] = tabla_segmentos[_A];
        display_buffer[0] = tabla_segmentos[_T];
    }
    // Desde aqui se muestran los frames de la primera animacion
    else if (strcmp(texto, "secuencia1") == 0)
    {
        display_buffer[2] = frame_1[0];
        display_buffer[1] = frame_1[1];
        display_buffer[0] = frame_1[2];
    }
    else if (strcmp(texto, "secuencia2") == 0)
    {
        display_buffer[2] = frame_2[0];
        display_buffer[1] = frame_2[1];
        display_buffer[0] = frame_2[2];
    }
    else if (strcmp(texto, "secuencia3") == 0)
    {
        display_buffer[2] = frame_3[0];
        display_buffer[1] = frame_3[1];
        display_buffer[0] = frame_3[2];
    }
    else if (strcmp(texto, "secuencia4") == 0)
    {
        display_buffer[2] = frame_4[0];
        display_buffer[1] = frame_4[1];
        display_buffer[0] = frame_4[2];
    }
    else if (strcmp(texto, "secuencia5") == 0)
    {
        display_buffer[2] = frame_5[0];
        display_buffer[1] = frame_5[1];
        display_buffer[0] = frame_5[2];
    }
    else if (strcmp(texto, "secuencia6") == 0)
    {
        display_buffer[2] = frame_6[0];
        display_buffer[1] = frame_6[1];
        display_buffer[0] = frame_6[2];
    }
    else if (strcmp(texto, "secuencia7") == 0)
    {
        display_buffer[2] = frame_7[0];
        display_buffer[1] = frame_7[1];
        display_buffer[0] = frame_7[2];
    }
    else if (strcmp(texto, "secuencia8") == 0)
    {
        display_buffer[2] = frame_8[0];
        display_buffer[1] = frame_8[1];
        display_buffer[0] = frame_8[2];
    }
    else if (strcmp(texto, "secuencia9") == 0)
    {
        display_buffer[2] = frame_9[0];
        display_buffer[1] = frame_9[1];
        display_buffer[0] = frame_9[2];
    }
    else if (strcmp(texto, "secuencia10") == 0)
    {
        display_buffer[2] = frame_10[0];
        display_buffer[1] = frame_10[1];
        display_buffer[0] = frame_10[2];
    }
    else if (strcmp(texto, "secuencia11") == 0)
    {
        display_buffer[2] = frame_11[0];
        display_buffer[1] = frame_11[1];
        display_buffer[0] = frame_11[2];
    }
    else if (strcmp(texto, "secuencia12") == 0)
    {
        display_buffer[2] = frame_12[0];
        display_buffer[1] = frame_12[1];
        display_buffer[0] = frame_12[2];
    }
    else if (strcmp(texto, "secuencia13") == 0)
    {
        display_buffer[2] = frame_13[0];
        display_buffer[1] = frame_13[1];
        display_buffer[0] = frame_13[2];
    }
    else if (strcmp(texto, "secuencia14") == 0)
    {
        display_buffer[2] = frame_14[0];
        display_buffer[1] = frame_14[1];
        display_buffer[0] = frame_14[2];
    }
}
