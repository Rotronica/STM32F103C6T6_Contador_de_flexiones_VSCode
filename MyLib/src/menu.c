#include "menu.h"
#include "button.h"
#include "buzzer.h"
#include "Display7seg.h"
#include "flexiones.h"
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
    menu->sub_config = SUB_UMBRAL_DOWN;

    // Activa configuracion secundarias
    menu->save_subconfig = OPCION_OBJETIVOS;
    menu->activar_subconfig = false;
    menu->activar_subconfig_umbral = false;
    menu->save_subconfig_umbral = SUB_UMBRAL_DOWN;
    menu->activar_ajuste_umbral = false;

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
    bool flags_starts = 0;
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
        flags_starts = button_is_pressed(menu->btn_start);
        // Empezar modo contador
        if (button_released(menu->btn_start) && flags_starts == 0)
        {
            menu->estado = MODO_CONTEO;
            menu->start_contador = true;
            buzzer_start(50);
        }
        // Entrar a modo Configuracion
        if (button_long_pressed(menu->btn_start) && flags_starts == 1)
        {
            menu->estado = MODO_CONFIGURACION;
            flags_starts = true;
            buzzer_start(100);
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
        if (!menu->activar_subconfig)
        {
            // Aqui se puede manipular btn up y down y navegar entre ociones
            // eso si es que la varible activar_subconfig es falso, cosa que cuando se navegue
            // y se seleccione cualquier opcion y se acepte con btn_start activa la varible: activar_subconfig=true
            // para luego bloquearlas y no interrumpa la subseleccion, en el cual de la misma manera
            // se utilizaran estos pulsadores up y down
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
        }
        // ir atras, se tiene que presionar unos segundos mas
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(200);
            menu->estado = MODO_ESPERA;
            menu->modo_config = OPCION_OBJETIVOS;
            menu->activar_subconfig = false;
            menu->activar_subconfig_umbral = false;
            menu->activar_ajuste_umbral = false;
        }
        // Para entrar al modo elegido con el boton start ya sea OPCION_OBJETIVO o OPCION_UMBRAL
        if (button_long_pressed(menu->btn_start) && !menu->activar_subconfig)
        {
            buzzer_start(200);
            menu->save_subconfig = menu->modo_config;
            menu->activar_subconfig = true;
        }
        if (menu->activar_subconfig)
        {
            switch (menu->save_subconfig)
            {
            case OPCION_OBJETIVOS:
                if (button_pressed(menu->btn_up) && menu->objetivo < 999)
                {
                    buzzer_start(100);
                    menu->objetivo++;
                }
                if (button_pressed(menu->btn_down) && menu->objetivo > 5)
                {
                    buzzer_start(100);
                    menu->objetivo--;
                }
                // Confirma objetivo de flexion
                if (button_pressed(menu->btn_start))
                {
                    buzzer_start(300);
                    flexiones_set_objetivo(menu->objetivo);
                    menu->activar_subconfig = false;
                }
                break;
            case OPCION_UMBRAL:
                menu->activar_subconfig_umbral = true;
                if (!menu->activar_ajuste_umbral)
                {
                    if (button_released(menu->btn_up))
                    {
                        buzzer_start(50);
                        if (menu->sub_config >= SUB_UMBRAL_UP)
                        {
                            menu->sub_config--;

                            if (menu->sub_config < 0) // Limete de umbral
                            {
                                menu->sub_config = 0;
                            }
                        }
                    }
                    if (button_released(menu->btn_down))
                    {
                        buzzer_start(50);
                        if (menu->sub_config <= SUB_UMBRAL_DOWN)
                        {
                            menu->sub_config++;
                            if (menu->sub_config > SUB_UMBRAL_UP)
                            {
                                menu->sub_config = SUB_UMBRAL_UP;
                            }
                        }
                    }
                }
                // Para entrar al modo elegido con el boton start SUB_UMBRAL_DOWN O UP
                if (button_pressed(menu->btn_start) && !menu->activar_ajuste_umbral)
                {
                    buzzer_start(200);
                    menu->save_subconfig_umbral = menu->sub_config;
                    menu->activar_ajuste_umbral = true;
                }
                if (menu->activar_ajuste_umbral)
                {
                    switch (menu->save_subconfig_umbral)
                    {
                    case SUB_UMBRAL_DOWN:
                        if (button_pressed(menu->btn_up) && menu->umbral_abajo < 999)
                        {
                            buzzer_start(100);
                            menu->umbral_abajo++;
                        }
                        if (button_pressed(menu->btn_down) && menu->umbral_abajo > 5)
                        {
                            buzzer_start(100);
                            menu->umbral_abajo--;
                        }
                        // Confirma umbral abajo
                        // if (button_released(menu->btn_start))
                        //{
                        // ✅ CORRECTO
                        uint32_t tiempo_espera_1 = button_pressed_duration(menu->btn_start);
                        if (tiempo_espera_1 >= 1000)
                        {
                            buzzer_start(1000);
                            flexion_umbral_bajo(menu->umbral_abajo);
                            menu->activar_ajuste_umbral = false;
                            menu->activar_subconfig = false;
                            menu->activar_subconfig_umbral = false;
                        }
                        //}
                        break;
                    case SUB_UMBRAL_UP:

                        if (button_pressed(menu->btn_up) && menu->umbral_arriba < 999)
                        {
                            buzzer_start(100);
                            menu->umbral_arriba++;
                        }
                        if (button_pressed(menu->btn_down) && menu->umbral_arriba > 5)
                        {
                            buzzer_start(100);
                            menu->umbral_arriba--;
                        }
                        // Confirma umbral alto
                        uint32_t tiempo_espera_2 = button_pressed_duration(menu->btn_start);
                        if (tiempo_espera_2 >= 1000)
                        {
                            buzzer_start(1000);
                            flexion_umbral_alto(menu->umbral_arriba);
                            menu->activar_ajuste_umbral = false;
                            menu->activar_subconfig = false;
                            menu->activar_subconfig_umbral = false;
                        }
                        break;

                    default:
                        break;
                    }
                }

                break;
            default:
                break;
            }
        }

        break;
    default:
        break;
    }
}

// Funcion para visualizar por display
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

            // Si se entro a la subconfiguracion
            if (menu->activar_subconfig == true)
            {
                Display7seg_show_number(menu->objetivo);
            }
            else
            {
                Display7seg_show_text("objetivo");
            }

            break;
        case OPCION_UMBRAL:
            // Necesito activar las subopciones
            if (menu->activar_subconfig_umbral)
            {
                switch (menu->sub_config)
                {
                case SUB_UMBRAL_UP:
                    if (menu->activar_ajuste_umbral)
                    {
                        Display7seg_show_number(menu->umbral_arriba);
                    }
                    else
                    {
                        Display7seg_show_text("arriba");
                    }

                    break;
                case SUB_UMBRAL_DOWN:
                    if (menu->activar_ajuste_umbral)
                    {
                        Display7seg_show_number(menu->umbral_abajo);
                    }
                    else
                    {
                        Display7seg_show_text("abajo");
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                Display7seg_show_text("umbral");
            }
            break;
        default:
            break;
        }
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
