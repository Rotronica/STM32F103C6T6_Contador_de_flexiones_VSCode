#include "adc.h"
#include "stm32f1xx_hal.h" // ← ESTE ES EL IMPORTANTE (incluye todo)

void ADC_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
}