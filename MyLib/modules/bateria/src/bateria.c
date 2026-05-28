#include "bateria.h"
#include "stm32f1xx_hal.h" // ← ESTE ES EL IMPORTANTE (incluye todo)
#include "adc.h"
#include "time_ticks.h"

static battery_config_t battery_config;
static battery_status_t battery_status;
static uint32_t last_read_time = 0;

void battery_init(battery_config_t *config)
{
    // Inicializando configuracion de la bateria
    battery_config.voltaje_max = config->voltaje_max;
    battery_config.voltaje_min = config->voltaje_min;
    battery_config.voltaje_advertencia = config->voltaje_advertencia;
    battery_config.divisor_tension = config->divisor_tension;

    // Inicializacion del estado de la bateria
    battery_status.adc_raw = 0;
    battery_status.voltaje_bateria = 0;
    battery_status.porcentaje = 0;
    battery_status.bateria_baja = false;
    battery_status.bateria_critica = false;

    last_read_time = 0;
}
void battery_update(battery_status_t *status_out)
{
    uint32_t ahora = millis();

    // Leer cada 1 segundo (no más frecuente)
    if (ahora - last_read_time < 1000)
    {
        if (status_out)
            *status_out = battery_status;
        return;
    }
    last_read_time = ahora;

    // Leer ADC usando el driver de CubeMX
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        battery_status.adc_raw = HAL_ADC_GetValue(&hadc1);

        // Calcular voltaje en el pin ADC (0-3.3V, 0-4095)
        float voltaje_adc = (battery_status.adc_raw * 3.30f) / 4095.00f;

        // Calcular voltaje real de la batería (aplicando divisor)
        battery_status.voltaje_bateria = voltaje_adc * battery_config.divisor_tension; // De 0 a 8.4V

        // Calcular porcentaje (mapeo lineal)
        float rango = battery_config.voltaje_max - battery_config.voltaje_min;
        if (rango > 0)
        {
            float porc = (battery_status.voltaje_bateria - battery_config.voltaje_min) / rango;
            if (porc < 0)
                porc = 0;
            if (porc > 1)
                porc = 1;
            battery_status.porcentaje = (uint8_t)(porc * 100);
        }
        else
        {
            battery_status.porcentaje = 0;
        }

        // Detectar niveles
        battery_status.bateria_baja = (battery_status.voltaje_bateria < battery_config.voltaje_advertencia);
        battery_status.bateria_critica = (battery_status.voltaje_bateria < battery_config.voltaje_min);
    }

    HAL_ADC_Stop(&hadc1);

    if (status_out)
        *status_out = battery_status;
}

uint8_t battery_get_percentage(void)
{
    return battery_status.porcentaje;
}

bool battery_is_low(void)
{
    return battery_status.bateria_baja;
}

bool battery_is_critical(void)
{
    return battery_status.bateria_critica;
}
