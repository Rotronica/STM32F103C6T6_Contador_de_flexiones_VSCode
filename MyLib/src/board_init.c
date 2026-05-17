#include "board_init.h"
#include "stm32f1xx_hal.h" // ← ESTE ES EL IMPORTANTE (incluye todo)
void hardware_init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    // Configuracion de los pines de salida PUERTO A
    GPIO_InitTypeDef config_gpios_Dpy = {
        .Pin = PIN_A1 | PIN_A2 | PIN_A3 | PIN_A3 | PIN_A4 | PIN_A5 | PIN_A6 | PIN_A7 | DISPLAY_1 | DISPLAY_2 | DISPLAY_3 | BUZZER_PIN,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    // Cargar la configuracion del pin a la funcion de configuracion
    HAL_GPIO_Init(GPIOA, &config_gpios_Dpy);
    // Colocar los pines en cero al iniciar
    HAL_GPIO_WritePin(GPIOA, PIN_A1 | PIN_A2 | PIN_A3 | PIN_A3 | PIN_A4 | PIN_A5 | PIN_A6 | PIN_A7 | DISPLAY_1 | DISPLAY_2 | DISPLAY_3 | BUZZER_PIN, GPIO_PIN_RESET);
    /*========Configuracion de los pines de salida Puerto B==========*/
    GPIO_InitTypeDef config_gpios_Btn = {
        .Pin = BUTTON_CONF_STR | BUTTON_DOWN | BUTTON_RESET | BUTTON_UP,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULLUP,
        //.Speed = NULL,
    };
    // Cargar la configuracion de los pines
    HAL_GPIO_Init(GPIOB, &config_gpios_Btn);
}