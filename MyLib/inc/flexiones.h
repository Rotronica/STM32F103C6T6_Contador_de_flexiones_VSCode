#ifndef FLEXIONES_H
#define FLEXIONES_H

#include <stdint.h>
#include <stdbool.h>

void flexiones_init(void);
uint16_t flexiones_actualizar(uint16_t distancia, uint16_t umbral_flexion, uint16_t umbral_arriba);
uint16_t flexiones_get_conteo(void);
void flexiones_set_conteo(uint16_t nuevo_conteo); // Para resetear o cargar valor

#endif