#pragma once

#include "button.h"
// Estructura enum para el estado del sistem
#define CANTIDAD_OPCIONES 3 // Es la cantidad de opciones que hay en modo configuracion
typedef enum
{
    // Modo
    MODO_ESPERA,
    MODO_CONTEO,
    MODO_CONFIGURACION,
    // Opciones disponibles en Modo configuracion
    OPCION_OBJETIVOS,
    OPCION_UMBRAL,
    OPCION_MEDICION,
    // Subopciones de la opcion UMBRAL
    UMBRAL_DOWN,
    UMBRAL_UP,
} sistema_estado_t;

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
    // Activa la configuraciones secundarias
    uint8_t save_subconfig; // Guarda las subopciones
    uint8_t save_umbrales;  // Guarda el tipo de umbral elegido

    // Valores configurables
    uint16_t objetivo;
    uint16_t umbral_abajo;
    uint16_t umbral_arriba;
    // Control de animación
    uint32_t last_animation;
    uint8_t animation_frame;

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
// Funcion para guardar datos de la funcion principal
void menu_read(uint16_t save_data);