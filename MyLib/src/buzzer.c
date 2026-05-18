#include "buzzer.h"
#include "time_ticks.h"
#include "board_init.h"
#include <stdbool.h>

static bool buzzer_activo = false;
static uint32_t tiempo_buzzer = 0;

void buzzer_init(void)
{
    buzzer_activo = false;
    tiempo_buzzer = 0;
}

// Cuando quieras activar el buzzer (ej: al completar una flexión)
void buzzer_start(void)
{
    buzzer_activo = true;
    tiempo_buzzer = millis(); // Guardar el momento de inicio
    HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_SET);
}

// En el main loop (llamar MUY frecuente)
void buzzer_update(void)
{
    if (buzzer_activo)
    {
        // Verificar si ya pasaron 2 segundos
        if (tick_espera(&tiempo_buzzer, TIEMPO_MS))
        {
            // ¡Se cumplió el tiempo!
            HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
            buzzer_activo = false; // Desactivar estado
        }
        // Si no se cumplió el tiempo, NO hacer nada (el buzzer sigue sonando)
    }
}