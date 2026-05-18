#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "time_ticks.h"

// Estructura principal del sensor
typedef struct
{
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
    uint8_t stop_variable;
    uint16_t timeout_ms;
    uint32_t measurement_start_time;
    bool is_measuring;
    bool last_timeout;
    uint8_t io_2v8;

    // Filtro de mediana
    uint16_t filter_buffer[5];
    uint8_t filter_index;
    uint8_t filter_count;
    uint16_t last_filtered;
} VL53L0X_t;

// ============================================
// FUNCIONES PRINCIPALES
// ============================================

bool VL53L0X_Init(VL53L0X_t *dev, I2C_HandleTypeDef *hi2c, bool io_2v8);
void VL53L0X_StartMeasurement(VL53L0X_t *dev);
bool VL53L0X_IsReady(VL53L0X_t *dev);
uint16_t VL53L0X_GetDistance(VL53L0X_t *dev);
uint16_t VL53L0X_GetRawDistance(VL53L0X_t *dev);
bool VL53L0X_TimeoutOccurred(VL53L0X_t *dev);
void VL53L0X_SetTimeout(VL53L0X_t *dev, uint16_t timeout_ms);

#endif