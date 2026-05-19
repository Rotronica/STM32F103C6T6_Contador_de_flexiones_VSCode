#include "menu.h"
#include "button.h"
#include "buzzer.h"
#include "Display7seg.h"
#define NUM_FRAMES 14

void menu_init(menu_t *menu,
               button_handle_t btn_start,
               button_handle_t btn_up,
               button_handle_t btn_down,
               button_handle_t btn_reset,
               uint16_t objetivo,
               uint16_t umbral_abajo,
               uint16_t umbral_arriba)
{
    // Guardar botones
    menu->btn_start = btn_start;
    menu->btn_reset = btn_reset;
    menu->btn_up = btn_up;
    menu->btn_down = btn_down;
    // Guardar estado
    menu->estado = MODO_ESPERA;
    menu->modo_config = OPCION_OBJETIVOS;
    menu->sub_config = SUB_UMBRAL_UP;
    menu->objetivo = objetivo;
    menu->umbral_abajo = umbral_abajo;
    menu->umbral_arriba = umbral_arriba;
    // Variables para la animacion de espera
    menu->last_animation = 0;
    menu->animation_frame = 0;
    // Condion inicial del contador si esta activado
    menu->start_contador = false;
    // Condicion inicial para el reset del contador
    menu->reset_contador = false;
}
void menu_update(menu_t *menu)
{
    uint32_t ahora = millis();
    switch (menu->estado)
    {
    case MODO_ESPERA:
        // Actualizar animación cada
        // Tiempo del frame 100ms
        if (ahora - menu->last_animation >= 100)
        {
            menu->last_animation = ahora;
            menu->animation_frame = (menu->animation_frame + 1) % NUM_FRAMES;
        }
        // 1. Verificar presión larga (entra a configuración)
        // Variables estáticas para detección manual
        static bool estaba_presionado = false;
        static uint32_t inicio_presion = 0;

        bool presionado_ahora = button_is_pressed(menu->btn_start);

        // Detectar flanco de bajada (comienza a presionar)
        if (presionado_ahora && !estaba_presionado)
        {
            inicio_presion = millis();
            estaba_presionado = true;
        }

        // Detectar flanco de subida (suelta)
        if (!presionado_ahora && estaba_presionado)
        {
            uint32_t duracion = millis() - inicio_presion;
            estaba_presionado = false;
            // Entrar a modo configuracion
            if (duracion >= 1000)
            {
                menu->estado = MODO_CONFIGURACION;
                buzzer_start(100);
            }
            // Entrar a modo conteo
            else if (duracion >= 50)
            {
                menu->estado = MODO_CONTEO;
                menu->start_contador = true;
                buzzer_start(50);
            }
        }

        break;
    case MODO_CONTEO:
        // Resetear contador
        if (button_pressed(menu->btn_reset))
        {
            buzzer_start(100);
            menu->reset_contador = true;
        }
        // ir atras, se tien que presionar unos segundos mas
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(300);
            menu->estado = MODO_ESPERA;
        }

        break;
    case MODO_CONFIGURACION:
        // Seleccion de las opciones
        if (button_released(menu->btn_up))
        {
            buzzer_start(50);
            if (menu->modo_config >= OPCION_UMBRAL)
            {
                menu->modo_config--;
                if (menu->modo_config < 0)
                {
                    menu->modo_config = 0;
                }
            }
        }
        if (button_released(menu->btn_down))
        {
            buzzer_start(50);
            if (menu->modo_config <= OPCION_OBJETIVOS)
            {
                menu->modo_config++;
                if (menu->modo_config > OPCION_UMBRAL)
                {
                    menu->modo_config = OPCION_UMBRAL;
                }
            }
        }
        // ir atras, se tien que presionar unos segundos mas
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(200);
            menu->estado = MODO_ESPERA;
            menu->modo_config = OPCION_OBJETIVOS;
        }

        break;
    default:
        break;
    }
}
sistema_estado_t menu_get_estado(menu_t *menu)
{
    // Retornar el estado segun la maquina de estados
    return menu->estado;
}
// Funciones para main
bool menu_cont_rst(menu_t *menu)
{
    bool estado_reset = menu->reset_contador;
    menu->reset_contador = false;
    return estado_reset;
}
bool menu_cont_str(menu_t *menu)
{
    bool estado_start = menu->start_contador;
    menu->start_contador = false;
    return estado_start;
}
void menu_update_display(menu_t *menu)
{
    switch (menu->estado)
    {
    case MODO_ESPERA:
        // Animación de espera (mostrar diferentes patrones)
        switch (menu->animation_frame)
        {
        case 0:
            Display7seg_show_text("secuencia1");
            break;
        case 1:
            Display7seg_show_text("secuencia2");
            break;
        case 2:
            Display7seg_show_text("secuencia3");
            break;
        case 3:
            Display7seg_show_text("secuencia4");
            break;
        case 4:
            Display7seg_show_text("secuencia5");
            break;
        case 5:
            Display7seg_show_text("secuencia6");
            break;
        case 6:
            Display7seg_show_text("secuencia7");
            break;
        case 7:
            Display7seg_show_text("secuencia8");
            break;
        case 8:
            Display7seg_show_text("secuencia9");
            break;
        case 9:
            Display7seg_show_text("secuencia10");
            break;
        case 10:
            Display7seg_show_text("secuencia11");
            break;
        case 11:
            Display7seg_show_text("secuencia12");
            break;
        case 12:
            Display7seg_show_text("secuencia13");
            break;
        case 13:
            Display7seg_show_text("secuencia14");
            break;
        default:
            Display7seg_show_number(0);
            break;
        }
        break;
    case MODO_CONTEO:
        // El contado se visualiza desde el main
        break;
    case MODO_CONFIGURACION:
        switch (menu->modo_config)
        {
        case OPCION_OBJETIVOS:
            Display7seg_show_text("objetivo");
            break;
        case OPCION_UMBRAL:
            // Para loas subopciones
            Display7seg_show_text("umbral");
            break;
        default:
            break;
        }
    default:
        break;
    }
}
