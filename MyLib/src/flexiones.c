#include "flexiones.h"
#include "buzzer.h" // Tu librería del buzzer
#include <stdint.h>

// Estados posibles del sistema
typedef enum
{
    ESTADO_ARRIBA,  // Posición inicial (pecho arriba)
    ESTADO_BAJANDO, // Está bajando hacia el suelo
    ESTADO_ABAJO,   // Flexión completada (pecho abajo)
    ESTADO_SUBIENDO // Está subiendo de vuelta
} estado_flexion_t;

// ============================================
// VARIABLES PRIVADAS
// ============================================
static uint16_t contador = 0;
static estado_flexion_t estado = ESTADO_ARRIBA;
static uint16_t distancia_anterior = 0;
static uint8_t contador_estable = 0;
static uint8_t lecturas_consecutivas = 0;

// Constantes
#define LECTURAS_PARA_CAMBIO 2   // Necesita 2 lecturas iguales para confirmar
#define DISTANCIA_MAX_VALIDA 300 // mm
#define DISTANCIA_MIN_VALIDA 50  // mm

#define TIEMPO_SIN_OBJETO 20 // 20 lecturas ≈ 1 segundo
static uint8_t contador_sin_objeto = 0;

static uint16_t objetivo_flexiones = 0;

// almacenar umbrales alto y bajo
uint16_t umbral_flexion = 0;
uint16_t umbral_arriba = 0;

// ============================================
// IMPLEMENTACIÓN
// ============================================

void flexiones_init(void)
{
    contador = 0;
    estado = ESTADO_ARRIBA;
    distancia_anterior = 0;
    contador_estable = 0;
    lecturas_consecutivas = 0;
    objetivo_flexiones = 10; // Limite de flexiones por defecto 10
    umbral_flexion = 50;     // Distancia en mm
    umbral_arriba = 200;     // Distancia en mm
}
uint16_t flexion_umbral_alto(uint16_t umbral_alto)
{
    return umbral_arriba = umbral_alto;
}
uint16_t flexion_umbral_bajo(uint16_t umbral_bajo)
{
    return umbral_flexion = umbral_bajo;
}
uint16_t flexiones_actualizar(uint16_t distancia)
{

    // ============================================
    // VALIDAR LECTURA
    // ============================================
    // Ignorar lecturas inválidas
    if (distancia == 0)
    {
        // Sin objeto detectado - contador de ausencia
        contador_sin_objeto++;
        if (contador_sin_objeto >= TIEMPO_SIN_OBJETO)
        {
            // Más de 1 segundo sin objeto: reiniciar estado
            if (estado != ESTADO_ARRIBA)
            {
                estado = ESTADO_ARRIBA;
                lecturas_consecutivas = 0;
                contador_estable = 0;
            }
        }
        return (uint8_t)contador;
    }
    else
    {
        contador_sin_objeto = 0;
    }

    // ============================================
    // MÁQUINA DE ESTADOS
    // ============================================
    switch (estado)
    {
    case ESTADO_ARRIBA:
        // Esperando que el usuario baje (distancia disminuye)
        if (distancia <= umbral_flexion)
        {
            estado = ESTADO_BAJANDO;
        }
        break;

    case ESTADO_BAJANDO:
        // Está bajando, esperar que llegue abajo
        if ((distancia <= umbral_flexion) && distancia > 0)
        {
            // Confirmar que sigue bajando
            if (distancia <= 100)
            {
                // Llegó muy abajo, considerar flexión completada
                estado = ESTADO_ABAJO;
            }
        }
        else if (distancia > umbral_arriba)
        {
            // Volvió a subir sin completar (falso positivo)
            estado = ESTADO_ARRIBA;
        }
        break;

    case ESTADO_ABAJO:
        // Flexión completada, esperando que suba
        if (distancia >= umbral_arriba)
        {
            estado = ESTADO_SUBIENDO;
            // contador_estable = 0;
        }
        break;

    case ESTADO_SUBIENDO:
        // Está subiendo, esperar que llegue arriba
        if (distancia >= umbral_arriba)
        {
            // Confirmar múltiples lecturas iguales para evitar rebotes
            if (distancia == distancia_anterior)
            {
                contador_estable++;
                if (contador_estable >= LECTURAS_PARA_CAMBIO)
                {
                    // ¡FLEXIÓN COMPLETA!
                    contador++;
                    // Limite de flexion
                    if (contador >= flexion_objetivo())
                    {
                        buzzer_start(2000);
                        contador = objetivo_flexiones;
                    }
                    else
                    {
                        // Activar buzzer (no bloqueante)
                        buzzer_start(500); // 500ms de pitido
                    }
                    estado = ESTADO_ARRIBA;
                    contador_estable = 0;
                }
            }
            else
            {
                contador_estable = 0;
            }
        }
        // Si vuelve a bajar sin completar (rebote)
        else if (distancia <= umbral_flexion)
        {
            estado = ESTADO_ABAJO;
            contador_estable = 0;
        }
        break;
    }

    distancia_anterior = distancia;
    return contador;
}
void flexiones_cont_reset()
{
    contador = 0;
}
uint16_t flexiones_get_conteo(void)
{
    return contador;
}

void flexiones_set_objetivo(uint16_t nuevo_objetivo)
{
    objetivo_flexiones = nuevo_objetivo;
}
uint16_t flexion_objetivo(void)
{
    return objetivo_flexiones;
}