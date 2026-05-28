#pragma once
#include "stm32f1xx_hal.h" // ← ESTE ES EL IMPORTANTE (incluye todo)
#include <stdint.h>
#include <stdbool.h>

// Creacion de atributos para la bateria
typedef struct
{
    float voltaje_max;         // 8.4V para 2S
    float voltaje_min;         // 6.0V para 2S
    float voltaje_advertencia; // 6.6V
    float divisor_tension;     // (R1+R2)/R2 del divisor de tensión
} battery_config_t;

// Atributos para mostrar elestado de la bateria
typedef struct
{
    uint16_t adc_raw;
    float voltaje_bateria;
    uint8_t porcentaje;
    bool bateria_baja;
    bool bateria_critica;
} battery_status_t;
void battery_init(battery_config_t *config);
void battery_update(battery_status_t *status_out);
uint8_t battery_get_percentage(void);
bool battery_is_low(void);
bool battery_is_critical(void);
