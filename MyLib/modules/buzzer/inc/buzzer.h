#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include <stdbool.h>

void buzzer_init(void);
void buzzer_start(uint16_t tiempo_ms); // Pitido único
void buzzer_stop(void);                // Apagar inmediato
void buzzer_update(void);              // Actualizar (llamar frecuentemente)

// Nueva función para pitidos intermitentes (alarma de batería baja)
void buzzer_alarm_start(uint16_t duracion_pitido_ms, uint8_t pitidos_por_ciclo, uint16_t intervalo_entre_ciclos_ms);
void buzzer_alarm_stop(void);
bool buzzer_alarm_is_active(void);

#endif