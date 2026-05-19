#pragma once
#include <stdint.h>
// Inicializamos pines
void buzzer_init(void);
// Cuando quieras activar el buzzer (ej: al completar una flexión)
void buzzer_start(uint16_t tiempo_ms);
// En el main loop (llamar MUY frecuente) es el que apaga el sonido

void buzzer_update(void);