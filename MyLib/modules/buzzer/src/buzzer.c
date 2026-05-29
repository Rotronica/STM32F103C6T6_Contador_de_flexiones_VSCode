#include "buzzer.h"
#include "time_ticks.h"
#include "board_init.h"
#include <stdbool.h>

// ============================================
// VARIABLES PARA PITIDO ÚNICO
// ============================================
static bool pitido_activo = false;
static uint32_t pitido_fin_time = 0;

// ============================================
// VARIABLES PARA ALARMA (pitidos intermitentes)
// ============================================
static bool alarma_activa = false;
static uint16_t alarma_duracion_pitido = 0;
static uint8_t alarma_pitidos_por_ciclo = 0;
static uint32_t alarma_intervalo_ciclos_ms = 0;

static uint8_t alarma_contador_pitidos = 0;
static bool alarma_estado_buzzer = false;
static uint32_t alarma_proxima_accion_time = 0;
static bool alarma_en_ciclo_pitidos = false;

//=============================================
// VARIABLE PITIDO PARA TOOGGLE
//=============================================
static buzzer_seq_t seq = {0};
// ============================================
// PITIDO ÚNICO
// ============================================

void buzzer_init(void)
{
    pitido_activo = false;
    alarma_activa = false;
    alarma_en_ciclo_pitidos = false;
    HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
}
void buzzer_seq_start(uint8_t num_pitidos, uint16_t duracion_ms, uint16_t pausa_ms)
{
    seq.activo = true;
    seq.pitidos_restantes = num_pitidos;
    seq.duracion_pitido = duracion_ms;
    seq.pausa_entre_pitidos = pausa_ms;
    seq.proxima_accion_time = millis();
    seq.pitido_encendido = false;
}
void buzzer_seq_update(void)
{
    if (!seq.activo)
        return;

    uint32_t ahora = millis();

    if (!seq.pitido_encendido && (ahora >= seq.proxima_accion_time))
    {
        // Encender pitido
        HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_SET);
        seq.pitido_encendido = true;
        seq.proxima_accion_time = ahora + seq.duracion_pitido;
    }
    else if (seq.pitido_encendido && (ahora >= seq.proxima_accion_time))
    {
        // Apagar pitido
        HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
        seq.pitido_encendido = false;
        seq.pitidos_restantes--;

        if (seq.pitidos_restantes > 0)
        {
            // Programar próximo pitido
            seq.proxima_accion_time = ahora + seq.pausa_entre_pitidos;
        }
        else
        {
            // Secuencia completa
            seq.activo = false;
        }
    }
}

void buzzer_start(uint16_t tiempo_ms)
{
    // Detener cualquier alarma activa
    buzzer_alarm_stop();

    pitido_activo = true;
    pitido_fin_time = millis() + tiempo_ms;
    HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_SET);
}

void buzzer_stop(void)
{
    pitido_activo = false;
    alarma_activa = false;
    alarma_en_ciclo_pitidos = false;
    HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
}

void buzzer_update(void)
{
    uint32_t ahora = millis();

    // ============================================
    // PRIORIDAD 1: Secuencia de pitidos (confirmaciones)
    // ============================================
    if (buzzer_seq_is_active())
    {
        buzzer_seq_update();
        return; // No procesar pitido único ni alarma mientras haya secuencia
    }

    // ============================================
    // PRIORIDAD 2: Pitido único
    // ============================================
    if (pitido_activo && ahora >= pitido_fin_time)
    {
        HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
        pitido_activo = false;
    }

    // ============================================
    // PRIORIDAD 3: Alarma (batería baja)
    // ============================================
    if (alarma_activa && ahora >= alarma_proxima_accion_time)
    {
        if (alarma_en_ciclo_pitidos)
        {
            // ... tu código de alarma existente ...
            if (alarma_contador_pitidos < alarma_pitidos_por_ciclo)
            {
                alarma_estado_buzzer = !alarma_estado_buzzer;
                HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, alarma_estado_buzzer);
                if (alarma_estado_buzzer)
                {
                    alarma_proxima_accion_time = ahora + alarma_duracion_pitido;
                }
                else
                {
                    alarma_contador_pitidos++;
                    alarma_proxima_accion_time = ahora + alarma_duracion_pitido;
                }
            }
            else
            {
                alarma_en_ciclo_pitidos = false;
                alarma_contador_pitidos = 0;
                alarma_estado_buzzer = false;
                alarma_proxima_accion_time = ahora + alarma_intervalo_ciclos_ms;
            }
        }
        else
        {
            alarma_en_ciclo_pitidos = true;
            alarma_contador_pitidos = 0;
            alarma_proxima_accion_time = ahora;
        }
    }
}
// ============================================
// ALARMA (PITIDOS INTERMITENTES)
// ============================================

void buzzer_alarm_start(uint16_t duracion_pitido_ms, uint8_t pitidos_por_ciclo, uint32_t intervalo_entre_ciclos_ms)
{
    // Detener cualquier pitido único en curso
    pitido_activo = false;

    // Configurar alarma
    alarma_activa = true;
    alarma_duracion_pitido = duracion_pitido_ms;
    alarma_pitidos_por_ciclo = pitidos_por_ciclo;
    alarma_intervalo_ciclos_ms = intervalo_entre_ciclos_ms;

    // Reiniciar estado
    alarma_contador_pitidos = 0;
    alarma_estado_buzzer = false;
    alarma_en_ciclo_pitidos = false;
    alarma_proxima_accion_time = millis(); // Comenzar inmediatamente
}

void buzzer_alarm_stop(void)
{
    alarma_activa = false;
    alarma_en_ciclo_pitidos = false;
    HAL_GPIO_WritePin(GPIOA, BUZZER_PIN, GPIO_PIN_RESET);
}

bool buzzer_alarm_is_active(void)
{
    return alarma_activa;
}
bool buzzer_seq_is_active(void)
{
    return seq.activo;
}