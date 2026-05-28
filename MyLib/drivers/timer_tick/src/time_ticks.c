#include "time_ticks.h"
#include "stm32f1xx_hal.h"

volatile uint32_t timer_tick = 0;

// Redefinir HAL_GetTick para que use timer_tick
uint32_t HAL_GetTick(void)
{
    return timer_tick;
}
// Anular la función débil original (evita que SysTick la incremente)
void HAL_IncTick(void)
{
    // No hacer nada - timer_tick ya se incrementa en TIM2
}
// Funcion de interrupcion
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timer_tick++;
    }
}
uint32_t millis(void)
{
    return timer_tick;
}

// Funcion utilizado para la espera sin bloqueo
bool tick_espera(uint32_t *referencia, uint32_t intervalo_ms)
{
    uint32_t ahora = timer_tick;
    if ((ahora - *referencia) >= intervalo_ms)
    {
        *referencia = ahora;
        return 1; // El tiempo paso
    }
    return 0; // El tiempo a un no paso
}