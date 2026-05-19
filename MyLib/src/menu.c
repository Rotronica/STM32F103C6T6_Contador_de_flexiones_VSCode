#include "menu.h"
#include "button.h"
#include "buzzer.h"
#include "Display7seg.h"
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

    // Condion inicial del contador si esta activado
    menu->start_contador = false;
    // Condicion inicial para el reset del contador
    menu->reset_contador = false;
}
void menu_update(menu_t *menu)
{
    static uint8_t selec_1 = 0; // Variable para seleccionar las dos opciones

    switch (menu->estado)
    {
    case MODO_ESPERA:
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

            if (duracion >= 1000)
            {
                menu->estado = MODO_CONFIGURACION;
                buzzer_start(100);
            }
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
            menu->reset_contador = true;
        }
        // ir atras, se tien que presionar unos segundos mas
        if (button_long_pressed(menu->btn_reset))
        {
            menu->estado = MODO_ESPERA;
        }

        break;
    case MODO_CONFIGURACION:
        // Seleccion de las opciones
        if (button_released(menu->btn_down) || button_released(menu->btn_up))
        {
            buzzer_start(50);
            if (selec_1 == 0)
            {
                menu->modo_config = OPCION_OBJETIVOS;
            }
            else if (selec_1 == 1)
            {
                menu->modo_config = OPCION_UMBRAL;
            }
            selec_1 = (selec_1 + 1) % 2;
        }
        // ir atras, se tien que presionar unos segundos mas
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(200);
            menu->estado = MODO_ESPERA;
            menu->modo_config = OPCION_OBJETIVOS;
            selec_1 = 0;
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
        Display7seg_show_text("inicio");
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
