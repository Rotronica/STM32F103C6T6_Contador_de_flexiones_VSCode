#include "buzzer.h"
#include "time_ticks.h"
#include "board_init.h"
#include <stdbool.h>

static bool buzzer_activo = false;
static uint32_t save_time = 0;
static uint16_t time_buzzer = 500;

void buzzer_init(void)
{
    buzzer_activo = false;
    save_time = 0;
    time_buzzer = 500;
}

// Cuando quieras activar el buzzer (ej: al completar una flexión)
void buzzer_start(uint16_t tiempo_ms)
{
    buzzer_activo = true;
    time_buzzer = tiempo_ms;
    save_time = millis(); // Guardar el momento de inicio
    HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_SET);
}

// En el main loop (llamar MUY frecuente)
void buzzer_update(void)
{
    if (buzzer_activo)
    {
        // Verificar si ya pasaron 2 segundos
        if (tick_espera(&save_time, time_buzzer))
        {
            // ¡Se cumplió el tiempo!
            HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
            buzzer_activo = false; // Desactivar estado
        }
        // Si no se cumplió el tiempo, NO hacer nada (el buzzer sigue sonando)
    }
}