/**
 * @file button.h
 * @brief Librería para manejo de pulsadores con antirrebote y detección de presión larga
 *
 * @details Esta librería proporciona una interfaz completa para manejar pulsadores
 *          en sistemas embebidos. Incluye filtro antirrebote por tiempo, detección
 *          de flancos (presión/liberación), detección de presión larga y medición
 *          de tiempo de presión. Está diseñada para ser 100% estática, sin uso
 *          de malloc, evitando fragmentación de memoria.
 *
 * @author Rodrigo Calle Condori
 * @version 2.0.0
 * @date 2026
 *
 * @note Requiere la función millis() disponible (proporcionada por time_ticks.h)
 * @note Los pines deben configurarse como entrada en CubeMX (con pull-up/pull-down según corresponda)
 * @note La función button_update_all() DEBE llamarse periódicamente (cada 10-20ms)
 *
 * @example
 * // Ejemplo de uso básico
 * button_init();
 *
 * button_config_t cfg = {
 *     .port = GPIOB, .pin = GPIO_PIN_0,
 *     .debounce_ms = 50, .long_press_ms = 1000,
 *     .pull_up = true, .active_low = true
 * };
 * button_handle_t btn = button_create(&cfg);
 *
 * while (1) {
 *     button_update_all();
 *     if (button_pressed(btn)) {
 *         // Acción al presionar
 *     }
 *     if (button_long_pressed(btn)) {
 *         // Acción por presión larga
 *     }
 * }
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "time_ticks.h"

/**
 * @brief Configuración de un pulsador
 *
 * @details Esta estructura contiene todos los parámetros necesarios
 *          para configurar un pulsador antes de crearlo.
 */
typedef struct
{
    GPIO_TypeDef *port;     /**< Puerto GPIO (ej: GPIOB, GPIOC) */
    uint16_t pin;           /**< Pin GPIO (ej: GPIO_PIN_0, GPIO_PIN_1) */
    uint32_t debounce_ms;   /**< Tiempo de antirrebote en milisegundos.
                                 @note Valor recomendado: 50ms */
    uint32_t long_press_ms; /**< Tiempo para considerar presión larga en milisegundos.
                                 @note Valor recomendado: 1000ms (1 segundo) */
    bool pull_up;           /**< Configuración de resistencia pull-up/pull-down.
                                 - true:  Habilita resistencia pull-up interna
                                 - false: Habilita resistencia pull-down interna
                                 @note Para botones que conectan a GND, usar true (pull-up) */
    bool active_low;        /**< Nivel lógico cuando el botón está presionado.
                                 - true:  Presionado = LOW (0V) [recomendado con pull-up]
                                 - false: Presionado = HIGH (3.3V) [recomendado con pull-down] */
} button_config_t;

/**
 * @brief Handle (identificador) de un botón
 *
 * @details Es un índice al pool interno de botones. Este enfoque evita
 *          el uso de punteros directos y malloc, previniendo fragmentación.
 *          Los handles se asignan secuencialmente al crear botones.
 *
 * @note El handle es un número entre 0 y BUTTON_MAX_BUTTONS-1
 * @note El valor BUTTON_INVALID_HANDLE indica un handle inválido
 */
typedef uint8_t button_handle_t;

/**
 * @brief Valor que indica un handle inválido
 *
 * @note button_create() retorna este valor si no hay espacio disponible
 */
#define BUTTON_INVALID_HANDLE 0xFF

/**
 * @brief Número máximo de botones que se pueden crear
 *
 * @details Este valor puede ser sobreescrito definiendo BUTTON_MAX_BUTTONS
 *          antes de incluir este archivo.
 *
 * @note Valor por defecto: 4 botones
 * @note Cada botón consume aproximadamente 28 bytes de RAM
 *
 * @example
 * #define BUTTON_MAX_BUTTONS 6
 * #include "button.h"
 */
#ifndef BUTTON_MAX_BUTTONS
#define BUTTON_MAX_BUTTONS 4
#endif

/**
 * @brief Inicializa la librería de pulsadores
 *
 * @details Debe llamarse UNA SOLA VEZ al inicio del programa, antes de
 *          crear cualquier botón. Limpia el pool de botones y prepara
 *          las estructuras internas.
 *
 * @note Esta función no configura los pines GPIO, solo inicializa el
 *       sistema interno de la librería. Los pines deben configurarse
 *       previamente (ej: con CubeMX o manualmente).
 *
 * @warning NO llamar esta función después de haber creado botones,
 *          ya que perdería las referencias.
 *
 * @example
 * int main() {
 *     HAL_Init();
 *     button_init();  // ← Llamar una vez al inicio
 *     // ... crear botones ...
 * }
 */
void button_init(void);

/**
 * @brief Crea y registra un nuevo pulsador
 *
 * @details Asigna un slot en el pool estático de botones e inicializa
 *          el estado interno del pulsador según la configuración.
 *          También configura el pin GPIO con la resistencia pull-up/pull-down.
 *
 * @param config Puntero a la estructura button_config_t con la configuración
 *
 * @return button_handle_t Handle del botón creado (0 a BUTTON_MAX_BUTTONS-1)
 * @retval BUTTON_INVALID_HANDLE Si no hay espacio disponible o los parámetros son inválidos
 *
 * @note El handle retornado es el índice del botón en el pool interno
 * @note Los handles se asignan secuencialmente del 0 al BUTTON_MAX_BUTTONS-1
 * @note Si se crean más botones que BUTTON_MAX_BUTTONS, la función falla
 *
 * @warning La configuración del pin sobreescribe cualquier configuración previa
 *
 * @example
 * button_config_t cfg = {
 *     .port = GPIOB,
 *     .pin = GPIO_PIN_0,
 *     .debounce_ms = 50,
 *     .long_press_ms = 1000,
 *     .pull_up = true,
 *     .active_low = true
 * };
 * button_handle_t btn = button_create(&cfg);
 * if (btn == BUTTON_INVALID_HANDLE) {
 *     // Error: no hay espacio
 * }
 */
button_handle_t button_create(const button_config_t *config);

/**
 * @brief Actualiza el estado de TODOS los botones
 *
 * @details Esta función debe llamarse periódicamente (idealmente cada 10-20ms)
 *          para actualizar el estado interno de todos los botones registrados.
 *          Lee el estado físico de los pines, aplica el filtro antirrebote,
 *          y actualiza los flags de presión, liberación y presión larga.
 *
 * @note Esta función es el corazón de la librería. Sin llamarla, las funciones
 *       button_pressed(), button_is_pressed(), etc., no funcionarán correctamente.
 * @note El tiempo entre llamadas debe ser menor que el tiempo de antirrebote
 *       para un correcto funcionamiento (recomendado: 10-20ms).
 *
 * @warning NO llamar esta función desde una interrupción (a menos que sea
 *          con prioridad baja) porque puede tomar varios microsegundos.
 *
 * @example
 * while (1) {
 *     button_update_all();  // ← Siempre al inicio del bucle
 *     Display_refresh();
 *     // ... resto del código ...
 * }
 */
void button_update_all(void);

/**
 * @brief Verifica si un botón fue presionado (flanco de bajada)
 *
 * @details Detecta el momento exacto en que el botón cambia de
 *          NO presionado a PRESIONADO. Retorna true solo UNA VEZ por presión,
 *          cuando ocurre el flanco. Es la función más común para acciones
 *          que deben ejecutarse una sola vez al presionar.
 *
 * @param handle Handle del botón (obtenido de button_create)
 *
 * @return bool
 * @retval true  El botón fue presionado en esta iteración
 * @retval false El botón no fue presionado
 *
 * @note El flag se limpia automáticamente después de ser leído
 * @note Recomendado para navegación en menús y acciones únicas
 *
 * @example
 * if (button_pressed(btn_enter)) {
 *     seleccionar_opcion();
 *     buzzer_start(50);
 * }
 */
bool button_pressed(button_handle_t handle);

/**
 * @brief Verifica si un botón fue liberado (flanco de subida)
 *
 * @details Detecta el momento exacto en que el botón cambia de
 *          PRESIONADO a NO presionado. Retorna true solo UNA VEZ por
 *          liberación, cuando ocurre el flanco. Útil para detectar
 *          el final de una presión larga.
 *
 * @param handle Handle del botón (obtenido de button_create)
 *
 * @return bool
 * @retval true  El botón fue liberado en esta iteración
 * @retval false El botón no fue liberado
 *
 * @note El flag se limpia automáticamente después de ser leído
 * @note Puede combinarse con button_pressed_duration() para detectar
 *       presión larga al soltar.
 *
 * @example
 * if (button_released(btn_start)) {
 *     uint32_t duracion = button_pressed_duration(btn_start);
 *     if (duracion >= 1000) {
 *         entrar_menu();  // Presión larga detectada al soltar
 *     }
 * }
 */
bool button_released(button_handle_t handle);

/**
 * @brief Verifica el estado actual de un botón (ya filtrado)
 *
 * @details Retorna true MIENTRAS el botón esté presionado, y false mientras
 *          esté liberado. El valor ya está filtrado por antirrebote.
 *
 * @param handle Handle del botón (obtenido de button_create)
 *
 * @return bool
 * @retval true  El botón está presionado en este momento
 * @retval false El botón está liberado
 *
 * @note Útil para acciones que deben repetirse mientras se mantiene presionado
 * @note No requiere limpiar flags (es un estado, no un evento)
 * @note Puede usarse con tick_espera() para crear autorepetición
 *
 * @example
 * // Sonido continuo mientras está presionado
 * if (button_is_pressed(btn_buzzer)) {
 *     buzzer_start(50);
 * }
 *
 * // Autorepetición cada 200ms
 * static uint32_t last = 0;
 * if (button_is_pressed(btn_up) && tick_espera(&last, 200)) {
 *     valor++;
 * }
 */
bool button_is_pressed(button_handle_t handle);

/**
 * @brief Verifica si hubo presión larga
 *
 * @details Detecta cuando el botón ha estado presionado durante más tiempo
 *          que el configurado en long_press_ms. Retorna true UNA SOLA VEZ
 *          cuando se alcanza el umbral de presión larga.
 *
 * @param handle Handle del botón (obtenido de button_create)
 *
 * @return bool
 * @retval true  Se detectó presión larga (presionado > long_press_ms)
 * @retval false No hay presión larga
 *
 * @note El flag se limpia automáticamente después de ser leído
 * @note No espera a que se suelte el botón, se activa al alcanzar el umbral
 * @note Ideal para funciones especiales como entrar a menú, reset, etc.
 *
 * @example
 * if (button_long_pressed(btn_start)) {
 *     entrar_menu_configuracion();
 *     buzzer_start(200);
 * }
 *
 * if (button_long_pressed(btn_reset)) {
 *     contador = 0;
 *     buzzer_start(100);
 * }
 */
bool button_long_pressed(button_handle_t handle);

/**
 * @brief Obtiene el tiempo que el botón lleva presionado
 *
 * @details Retorna la duración en milisegundos que el botón ha estado
 *          presionado (si está presionado). Si el botón está liberado,
 *          retorna 0. Útil para implementar comportamientos dependientes
 *          del tiempo de presión.
 *
 * @param handle Handle del botón (obtenido de button_create)
 *
 * @return uint32_t Tiempo de presión en milisegundos
 * @retval 0 Botón liberado o no presionado
 * @retval >0 Tiempo actual de presión
 *
 * @note Esta función NO limpia ningún flag
 * @note Puede usarse dentro de button_released() para saber cuánto tiempo
 *       estuvo presionado el botón.
 *
 * @example
 * // Detectar presión larga al soltar
 * if (button_released(btn_start)) {
 *     uint32_t duracion = button_pressed_duration(btn_start);
 *     if (duracion >= 2000) {
 *         entrar_configuracion();
 *     } else {
 *         iniciar_conteo();
 *     }
 * }
 *
 * // Mostrar tiempo de presión en display
 * if (button_is_pressed(btn_test)) {
 *     Display7seg_show_number(button_pressed_duration(btn_test));
 * }
 */
uint32_t button_pressed_duration(button_handle_t handle);
/**
 * @brief Detecta acción del botón al soltar (corta vs larga)
 * @param handle Handle del botón
 * @param umbral_ms Tiempo en ms para considerar presión larga (ej: 800)
 * @return 0 = sin acción, 1 = presión corta, 2 = presión larga
 */
uint8_t button_action(button_handle_t handle, uint32_t umbral_ms);

#endif /* BUTTON_H */