#include "Display7seg.h"
#include "board_init.h"
#include "time_ticks.h"

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
};
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
void Display_refresh(void)
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
void Display7_show_number(uint16_t numero)
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
