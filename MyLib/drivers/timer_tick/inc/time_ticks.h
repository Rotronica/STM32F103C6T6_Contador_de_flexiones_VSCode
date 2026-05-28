#pragma once
#include <stdint.h>
#include <stdbool.h>
bool tick_espera(uint32_t *referencia, uint32_t intervalo_ms);
uint32_t millis(void);