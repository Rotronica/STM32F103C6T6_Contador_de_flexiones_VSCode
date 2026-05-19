#pragma once

#include "button.h"
// Estructura enum para el estado del sistem
typedef enum
{
    MODO_ESPERA,
    MODO_CONTEO,
    MODO_CONFIGURACION,
} sistema_estado_t;
// Estructura enum para las opciones cuando se ingrese a configuracon
typedef enum
{
    OPCION_OBJETIVOS,
    OPCION_UMBRAL,
} modo_config_t;
// Estructura para las suboconfiguraciones cuando se selecciona una OPCION_UMBRAL
typedef enum
{
    SUB_UMBRAL_UP,
    SUB_UMBRAL_DOWN,
} sub_config_t;

// Atributos para el menu sera utilizado para guardar las configuracion
typedef struct
{
    // Botones a usar para el menu
    button_handle_t btn_start;
    button_handle_t btn_reset;
    button_handle_t btn_up;
    button_handle_t btn_down;

    // Estado
    sistema_estado_t estado;
    // Modo Configuracion
    modo_config_t modo_config;
    // Modo subconfiguraciones
    sub_config_t sub_config;
    // Valores configurables
    uint16_t objetivo;
    uint16_t umbral_abajo;
    uint16_t umbral_arriba;

    // Reset del contador
    bool start_contador;
    bool reset_contador;
} menu_t;

void menu_init(menu_t *menu,
               button_handle_t btn_start,
               button_handle_t btn_up,
               button_handle_t btn_down,
               button_handle_t btn_reset,
               uint16_t objetivo,
               uint16_t umbral_abajo,
               uint16_t umbral_arriba);
// Logica de negocios Procesa botones, cambia estados, actualiza valores
void menu_update(menu_t *menu);
// Funcion que devuelve un verdadero o falso si el reset del contado fuen presionado
bool menu_cont_rst(menu_t *menu);
// Funcion que devuelce true o false si se actico el modo conteo
bool menu_cont_str(menu_t *menu);
// Funcion que devueve el estado en que esta nuestrao sistema
sistema_estado_t menu_get_estado(menu_t *menu);
// Visualizador de menus
void menu_update_display(menu_t *menu);