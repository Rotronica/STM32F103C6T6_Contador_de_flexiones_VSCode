#include "time_ticks.h"
volatile uint32_t timer_tick = 0;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    // HAL_GPIO_TogglePin(GPIOA, PIN_A1);
    if (htim->Instance == TIM2)
    {
        timer_tick++;
    }
}
uint8_t tick_espera(uint32_t *referencia, uint32_t intervalo_ms)
{
    uint32_t ahora = timer_tick;
    if ((ahora - *referencia) >= intervalo_ms)
    {
        *referencia = ahora;
        return 1; // El tiempo paso
    }
    return 0; // El tiempo a un no paso
}