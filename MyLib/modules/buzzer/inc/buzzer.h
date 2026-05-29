#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include <stdbool.h>
typedef struct
{
    bool activo;
    uint8_t pitidos_restantes;
    uint16_t duracion_pitido;
    uint16_t pausa_entre_pitidos;
    uint32_t proxima_accion_time;
    bool pitido_encendido;
} buzzer_seq_t;

void buzzer_init(void);
void buzzer_start(uint16_t tiempo_ms); // Pitido único
void buzzer_stop(void);                // Apagar inmediato
void buzzer_update(void);              // Actualizar (llamar frecuentemente)

// Nueva función para pitidos intermitentes (alarma de batería baja)
void buzzer_alarm_start(uint16_t duracion_pitido_ms, uint8_t pitidos_por_ciclo, uint32_t intervalo_entre_ciclos_ms);
void buzzer_alarm_stop(void);
bool buzzer_alarm_is_active(void);

// Funciones para el pitido tooggle
void buzzer_seq_start(uint8_t num_pitidos, uint16_t duracion_ms, uint16_t pausa_ms);
void buzzer_seq_update(void);
bool buzzer_seq_is_active(void);
#endif