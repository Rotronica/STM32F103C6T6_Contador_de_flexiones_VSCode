/**
 * @file VL53L0X.h
 * @brief Librería para el sensor de distancia VL53L0X (Time-of-Flight)
 * @author Rodrigo Calle Condori
 * @version 1.0.0
 * @date 2026
 *
 * @details Esta librería proporciona una interfaz no bloqueante para el sensor
 *          de distancia VL53L0X. Utiliza la función millis() basada en TIM2
 *          para manejar timeouts sin bloquear el sistema.
 *
 * @note El sensor debe estar alimentado a 3.3V (NO 5V)
 * @note Requiere resistencias pull-up de 4.7kΩ en líneas SCL y SDA
 */

#ifndef VL53L0X_H
#define VL53L0X_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "time_ticks.h"

/**
 * @brief Estructura principal del sensor VL53L0X
 *
 * Contiene toda la información de estado, configuración y filtros
 * para un sensor. Permite usar múltiples sensores en el mismo bus I2C.
 */
typedef struct
{
    I2C_HandleTypeDef *hi2c;         /**< Puntero al handle de I2C (ej: &hi2c1) */
    uint8_t address;                 /**< Dirección I2C del sensor (0x52 por defecto) */
    uint8_t stop_variable;           /**< Variable interna del sensor para mediciones */
    uint16_t timeout_ms;             /**< Tiempo máximo de espera para medición (ms) */
    uint32_t measurement_start_time; /**< Momento de inicio de la medición actual */
    bool is_measuring;               /**< Flag: true si hay una medición en curso */
    bool last_timeout;               /**< Flag: true si la última medición expiró */
    uint8_t io_2v8;                  /**< Configuración de voltaje (1 para 2.8V, 0 para 1.8V) */

    /* Filtro de mediana (para suavizar lecturas) */
    uint16_t filter_buffer[5]; /**< Buffer circular para últimas 5 mediciones */
    uint8_t filter_index;      /**< Índice actual en el buffer circular */
    uint8_t filter_count;      /**< Número de mediciones acumuladas (máx 5) */
    uint16_t last_filtered;    /**< Último valor filtrado por la mediana */
} VL53L0X_t;

/**
 * @brief Inicializa el sensor VL53L0X con la configuración completa
 *
 * @details Realiza toda la secuencia de inicialización incluyendo:
 *          - Configuración de voltaje (1.8V/2.8V)
 *          - Calibración de SPAD (Single Photon Avalanche Diode)
 *          - Calibración VHV y de fase
 *          - Configuración de interrupciones
 *          - Inicialización del filtro de mediana
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 * @param hi2c  Puntero al handle de I2C (ej: &hi2c1)
 * @param io_2v8 Configuración de voltaje:
 *               - true:  Modo 2.8V (recomendado para la mayoría de módulos)
 *               - false: Modo 1.8V (bajo consumo)
 *
 * @return bool
 *         - true:  Inicialización exitosa, sensor listo para usar
 *         - false: Error en la inicialización (verificar conexiones)
 *
 * @note Esta función debe llamarse UNA SOLA VEZ al inicio del programa
 * @note El sensor debe estar correctamente conectado antes de llamar esta función
 *
 * @example
 * static VL53L0X_t sensor;
 * if (VL53L0X_Init(&sensor, &hi2c1, true)) {
 *     // Sensor listo para usar
 * }
 */
bool VL53L0X_Init(VL53L0X_t *dev, I2C_HandleTypeDef *hi2c, bool io_2v8);

/**
 * @brief Inicia una medición de distancia (modo no bloqueante)
 *
 * @details Envía el comando al sensor para iniciar una nueva medición.
 *          Esta función retorna inmediatamente sin esperar el resultado.
 *          El estado de la medición debe verificarse con VL53L0X_IsReady().
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 * @param time_ms   Tiempo ms que le toma para medir
 *
 * @note Si ya hay una medición en curso, esta función no hace nada
 * @note Después de llamar esta función, se debe verificar IsReady() periódicamente
 *
 * @example
 * VL53L0X_StartMeasurement(&sensor);
 * // Continuar con otras tareas mientras el sensor mide
 */
void VL53L0X_StartMeasurement(VL53L0X_t *dev, uint16_t time_ms);

/**
 * @brief Verifica si la medición actual ya está completa
 *
 * @details Comprueba si el sensor ha terminado la medición actual.
 *          También maneja el timeout: si la medición tarda más de
 *          timeout_ms, se considera fallida.
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 *
 * @return bool
 *         - true:  La medición está completa (lista para leer)
 *         - false: La medición aún está en curso
 *
 * @note Esta función debe llamarse periódicamente (ej: cada 5-10ms)
 * @note La función NO es bloqueante, retorna inmediatamente
 *
 * @example
 * if (VL53L0X_IsReady(&sensor)) {
 *     uint16_t dist = VL53L0X_GetDistance(&sensor);
 * }
 */
bool VL53L0X_IsReady(VL53L0X_t *dev);

/**
 * @brief Obtiene la distancia filtrada (recomendada)
 *
 * @details Lee el resultado de la medición y aplica un filtro de mediana
 *          con las últimas 5 lecturas para eliminar ruido y lecturas espurias.
 *          El resultado es más estable que GetRawDistance().
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @return uint16_t
 *         - Rango válido: 20 a 1200 (milímetros)
 *         - 20:   Distancia mínima detectable (muy cerca)
 *         - 1200: Distancia máxima (1200mm = 1.2m)
 *         - Valor anterior: Si la lectura es inválida, mantiene el último válido
 *
 * @note Solo debe llamarse DESPUÉS de que VL53L0X_IsReady() retorne true
 * @note Limpia automáticamente la interrupción del sensor
 *
 * @example
 * if (VL53L0X_IsReady(&sensor)) {
 *     uint16_t dist = VL53L0X_GetDistance(&sensor);
 *     if (dist < 100) {
 *         // Objeto muy cerca
 *     }
 * }
 */
uint16_t VL53L0X_GetDistance(VL53L0X_t *dev);

/**
 * @brief Obtiene la distancia cruda (sin filtro)
 *
 * @details Lee el resultado de la medición directamente del sensor,
 *          sin aplicar ningún filtro. Útil para depuración o si se necesita
 *          la lectura más rápida posible.
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @return uint16_t
 *         - Rango válido: 20 a 1200 (milímetros)
 *         - 65535: Error de timeout
 *
 * @warning Las lecturas crudas pueden contener ruido y valores espurios
 * @note Solo debe llamarse DESPUÉS de que VL53L0X_IsReady() retorne true
 * @note Limpia automáticamente la interrupción del sensor
 */
uint16_t VL53L0X_GetRawDistance(VL53L0X_t *dev);

/**
 * @brief Verifica si la última medición tuvo timeout
 *
 * @details Indica si la medición anterior expiró por timeout.
 *          Esto puede ocurrir si el sensor no responde o no hay objeto detectable.
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @return bool
 *         - true:  La última medición expiró por timeout
 *         - false: La última medición fue exitosa
 *
 * @note El flag se limpia automáticamente después de leerlo
 * @note Usar después de VL53L0X_IsReady() y antes de GetDistance()
 *
 * @example
 * if (VL53L0X_IsReady(&sensor)) {
 *     if (VL53L0X_TimeoutOccurred(&sensor)) {
 *         Display7_show_number(888);  // Error
 *     } else {
 *         uint16_t dist = VL53L0X_GetDistance(&sensor);
 *     }
 * }
 */
bool VL53L0X_TimeoutOccurred(VL53L0X_t *dev);

/**
 * @brief Configura el tiempo de timeout para las mediciones
 *
 * @details Establece el tiempo máximo que se esperará por una medición
 *          antes de considerarla fallida. Aumentar este valor puede ayudar
 *          en superficies poco reflectantes o largas distancias.
 *
 * @param dev         Puntero a la estructura VL53L0X_t del sensor
 * @param timeout_ms  Tiempo de timeout en milisegundos
 *                    - 50:   Valor por defecto (rápido)
 *                    - 100:  Recomendado para mayor confiabilidad
 *                    - 200:  Para distancias largas o superficies oscuras
 *
 * @note El timeout debe ser mayor que el tiempo de medición del sensor (~33ms)
 * @note Se recomienda un valor entre 50ms y 200ms
 *
 * @example
 * VL53L0X_SetTimeout(&sensor, 100);  // Timeout de 100ms
 */
void VL53L0X_SetTimeout(VL53L0X_t *dev, uint16_t timeout_ms);

#endif /* VL53L0X_H */