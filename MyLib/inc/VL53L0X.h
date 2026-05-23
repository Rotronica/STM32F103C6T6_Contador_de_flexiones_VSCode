/**
 * @file VL53L0X.h
 * @brief Librería para el sensor de distancia VL53L0X (Time-of-Flight)
 *
 * @details Esta librería proporciona una interfaz no bloqueante para el sensor
 *          de distancia VL53L0X. Incluye filtro de mediana, modo High Speed
 *          para detección de movimiento rápido, y manejo de timeouts.
 *
 * @author Rodrigo Calle Condori
 * @version 2.0.0
 * @date 2026
 *
 * @note El sensor debe estar alimentado a 3.3V (NO 5V)
 * @note Requiere resistencias pull-up de 4.7kΩ en líneas SCL y SDA
 * @note Requiere la función millis() disponible (time_ticks.h)
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
 * @details Contiene toda la información de estado, configuración y filtros
 *          para un sensor. Permite usar múltiples sensores en el mismo bus I2C.
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

    /* Control de temporización independiente */
    uint32_t last_measurement_time; /**< Tiempo de la última medición iniciada */

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
 */
bool VL53L0X_Init(VL53L0X_t *dev, I2C_HandleTypeDef *hi2c, bool io_2v8);

/**
 * @brief Configura el sensor en modo High Speed (20ms por medición)
 *
 * @details Reduce el timing budget a 20ms y ajusta los periodos VCSEL
 *          para máxima velocidad. Ideal para detección de movimiento rápido.
 *          La precisión se reduce ligeramente a ±5%.
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @note Tiempo de medición: ~20ms
 * @note Rango máximo: ~1.2m
 *
 * @example
 * VL53L0X_Init(&sensor, &hi2c1, true);
 * VL53L0X_SetHighSpeedMode(&sensor);  // Para flexiones rápidas
 */
void VL53L0X_SetHighSpeedMode(VL53L0X_t *dev);

/**
 * @brief Configura el sensor en modo por defecto (33ms por medición)
 *
 * @details Configuración estándar con balance entre velocidad y precisión.
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @note Tiempo de medición: ~33ms
 * @note Precisión: ±3%
 */
void VL53L0X_SetDefaultMode(VL53L0X_t *dev);

/**
 * @brief Inicia una medición de distancia (modo no bloqueante)
 *
 * @details Envía el comando al sensor para iniciar una nueva medición.
 *          Esta función retorna inmediatamente sin esperar el resultado.
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @note Si ya hay una medición en curso, esta función no hace nada
 *
 * @example
 * VL53L0X_StartMeasurement(&sensor);
 */
void VL53L0X_StartMeasurement(VL53L0X_t *dev);

/**
 * @brief Inicia una medición solo si ha pasado el intervalo especificado
 *
 * @details Función útil para controlar la frecuencia de medición desde el bucle principal.
 *
 * @param dev          Puntero a la estructura VL53L0X_t del sensor
 * @param interval_ms  Intervalo mínimo entre mediciones (ms)
 *
 * @return bool
 *         - true:  Se inició una nueva medición
 *         - false: No se inició (en curso o intervalo no cumplido)
 *
 * @example
 * if (VL53L0X_StartMeasurementIfReady(&sensor, 25)) {
 *     // Medición iniciada
 * }
 */
bool VL53L0X_StartMeasurementIfReady(VL53L0X_t *dev, uint32_t interval_ms);

/**
 * @brief Verifica si la medición actual ya está completa
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @return bool
 *         - true:  La medición está completa (lista para leer)
 *         - false: La medición aún está en curso
 *
 * @note Esta función debe llamarse periódicamente (ej: cada 5-10ms)
 * @note La función NO es bloqueante, retorna inmediatamente
 */
bool VL53L0X_IsReady(VL53L0X_t *dev);

/**
 * @brief Obtiene la distancia filtrada (recomendada)
 *
 * @details Lee el resultado de la medición y aplica un filtro de mediana
 *          con las últimas 5 lecturas para eliminar ruido.
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @return uint16_t
 *         - Rango válido: 20 a 1200 (milímetros)
 *         - 20:   Distancia mínima detectable
 *         - 1200: Distancia máxima (1200mm = 1.2m)
 *
 * @note Solo debe llamarse DESPUÉS de que VL53L0X_IsReady() retorne true
 */
uint16_t VL53L0X_GetDistance(VL53L0X_t *dev);

/**
 * @brief Obtiene la distancia cruda (sin filtro)
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @return uint16_t Distancia en mm (65535 si error)
 *
 * @warning Las lecturas crudas pueden contener ruido
 */
uint16_t VL53L0X_GetRawDistance(VL53L0X_t *dev);

/**
 * @brief Verifica si la última medición tuvo timeout
 *
 * @param dev   Puntero a la estructura VL53L0X_t del sensor
 *
 * @return bool
 *         - true:  La última medición expiró
 *         - false: La última medición fue exitosa
 *
 * @note El flag se limpia automáticamente después de leerlo
 */
bool VL53L0X_TimeoutOccurred(VL53L0X_t *dev);

/**
 * @brief Configura el tiempo de timeout para las mediciones
 *
 * @param dev         Puntero a la estructura VL53L0X_t del sensor
 * @param timeout_ms  Tiempo de timeout en milisegundos
 *
 * @note Recomendado: 30ms para High Speed, 50ms para Default
 */
void VL53L0X_SetTimeout(VL53L0X_t *dev, uint16_t timeout_ms);

#endif /* VL53L0X_H */