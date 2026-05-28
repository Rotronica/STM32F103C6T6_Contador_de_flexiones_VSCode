#ifndef FLEXIONES_H
#define FLEXIONES_H

#include <stdint.h>
#include <stdbool.h>

void flexiones_init(void);
uint16_t flexiones_actualizar(uint16_t distancia);
uint16_t flexiones_get_conteo(void);
void flexiones_set_objetivo(uint16_t nuevo_objetivo); // Para setear el nuevo objetivo
uint16_t flexion_objetivo(void);
uint16_t flexion_umbral_alto(uint16_t umbral_alto);
uint16_t flexion_umbral_bajo(uint16_t umbral_bajo);
void flexiones_cont_reset();

#endif