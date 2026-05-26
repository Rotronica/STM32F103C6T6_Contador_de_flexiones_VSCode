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

    // Activa configuracion secundarias
    menu->save_subconfig = OPCION_OBJETIVOS;
    menu->save_umbrales = UMBRAL_UP;

    menu->objetivo = objetivo;
    flexiones_set_objetivo(objetivo);

    menu->umbral_abajo = umbral_abajo;
    menu->umbral_arriba = umbral_arriba;
    flexion_umbral_alto(umbral_arriba);
    flexion_umbral_bajo(umbral_abajo);
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
        // Empezar modo contador
        if (button_released(menu->btn_start))
        {
            menu->estado = MODO_CONTEO;
            menu->start_contador = true;
            buzzer_start(100);
        }
        // Se presiono largo tiempo ingresa a modo configuracion
        if (button_long_pressed(menu->btn_start))
        {
            menu->estado = MODO_CONFIGURACION;
            buzzer_start(300);
        }

        break;
    case MODO_CONTEO:
        // Resetear contador
        if (button_released(menu->btn_reset))
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

        // Aqui se puede manipular btn up y down y navegar entre ociones
        // eso si es que la varible activar_subconfig es falso, cosa que cuando se navegue
        // y se seleccione cualquier opcion y se acepte con btn_start activa la varible: activar_subconfig=true
        // para luego bloquearlas y no interrumpa la subseleccion, en el cual de la misma manera
        // se utilizaran estos pulsadores up y down
        static uint8_t opciones = 1;
        if (button_released(menu->btn_up))
        {
            buzzer_start(50);
            opciones = opciones + 1;
            if (opciones >= 2)
            {
                opciones = 2;
            }
        }
        if (button_released(menu->btn_down))
        {
            buzzer_start(50);
            opciones = opciones - 1;
            if (opciones <= 1)
            {
                opciones = 1;
            }
        }
        switch (opciones)
        {
        case 1:
            menu->save_subconfig = OPCION_OBJETIVOS;
            break;
        case 2:
            menu->save_subconfig = OPCION_UMBRAL;
            break;
        default:
            break;
        }
        // Confirmar opcion elegida
        if (button_long_pressed(menu->btn_start))
        {
            buzzer_start(300);
            menu->estado = menu->save_subconfig;
        }
        // ir atras, se tiene que presionar unos segundos mas
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(300);
            menu->estado = MODO_ESPERA;
            menu->save_subconfig = OPCION_OBJETIVOS;
            opciones = 1; // Variable de seleccion Condicion incial de las opciones Se coloca a OPCION_OBJETIVOS
        }
        break;
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
        // Guardar objetivos configuraciones
        if (button_long_pressed(menu->btn_start))
        {
            buzzer_start(300);
            flexiones_set_objetivo(menu->objetivo);
        }
        // Reset
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(200);
            menu->estado = MODO_ESPERA;
            menu->save_subconfig = OPCION_OBJETIVOS;
        }
        break;
    case OPCION_UMBRAL:
        // Opciones a escojer
        if (button_released(menu->btn_up))
        {
            buzzer_start(50);
            menu->save_umbrales = UMBRAL_UP;
        }
        if (button_released(menu->btn_down))
        {
            buzzer_start(50);
            menu->save_umbrales = UMBRAL_DOWN;
        }
        // Ingresar a la opcion elegida
        if (button_long_pressed(menu->btn_start))
        {
            buzzer_start(300);
            menu->estado = menu->save_umbrales;
        }

        // Reset
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(200);
            menu->estado = MODO_ESPERA;
            menu->save_subconfig = OPCION_OBJETIVOS;
            menu->save_umbrales = UMBRAL_UP;
            opciones = 1; // Variable de seleccion Condicion incial de las opciones Se coloca a OPCION_OBJETIVOS
        }
        break;
    case UMBRAL_UP:
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
        // Guardar configuuracion
        if (button_long_pressed(menu->btn_start))
        {
            buzzer_start(300);
            flexion_umbral_alto(menu->umbral_arriba);
        }
        // Reset
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(200);
            menu->estado = MODO_ESPERA;
            menu->save_subconfig = OPCION_OBJETIVOS;
            menu->save_umbrales = UMBRAL_UP;
            opciones = 1; // Variable de seleccion Condicion incial de las opciones Se coloca a OPCION_OBJETIVOS
        }
        break;
    case UMBRAL_DOWN:
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
        // Reset
        if (button_long_pressed(menu->btn_reset))
        {
            buzzer_start(200);
            menu->estado = MODO_ESPERA;
            menu->save_subconfig = OPCION_OBJETIVOS;
            menu->save_umbrales = UMBRAL_UP;
            opciones = 1; // Variable de seleccion Condicion incial de las opciones Se coloca a OPCION_OBJETIVOS
        }
        // Guardar configuuracion
        if (button_long_pressed(menu->btn_start))
        {
            buzzer_start(300);
            flexion_umbral_bajo(menu->umbral_abajo);
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
        if (menu->save_subconfig == OPCION_OBJETIVOS)
        {
            Display7seg_show_text("objetivo");
        }
        if (menu->save_subconfig == OPCION_UMBRAL)
        {
            Display7seg_show_text("umbral");
        }
        break;
    case OPCION_OBJETIVOS:
        Display7seg_show_number(menu->objetivo);
        break;
    case OPCION_UMBRAL:
        if (menu->save_umbrales == UMBRAL_UP)
        {
            Display7seg_show_text("arriba");
        }
        if (menu->save_umbrales == UMBRAL_DOWN)
        {
            Display7seg_show_text("abajo");
        }
        break;
    case UMBRAL_UP:
        Display7seg_show_number(menu->umbral_arriba);
        break;
    case UMBRAL_DOWN:
        Display7seg_show_number(menu->umbral_abajo);
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
