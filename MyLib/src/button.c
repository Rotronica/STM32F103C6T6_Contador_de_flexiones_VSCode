#include "button.h"

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    uint32_t debounce_ms;
    uint32_t long_press_ms;
    bool active_low;

    bool estado_filtrado;
    uint32_t ultimo_cambio;
    uint32_t inicio_presion;
    bool flag_presionado;
    bool flag_liberado;
    bool flag_long_press;
    bool long_press_triggered;
    bool estado_anterior;
} button_internal_t;

static button_internal_t buttons[BUTTON_MAX_BUTTONS];
static bool button_used[BUTTON_MAX_BUTTONS] = {false};
static uint8_t button_count = 0;

static bool button_raw_read(button_internal_t *btn)
{
    bool estado = (HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_RESET);
    if (!btn->active_low)
        estado = !estado;
    return estado;
}

void button_init(void)
{
    for (int i = 0; i < BUTTON_MAX_BUTTONS; i++)
    {
        buttons[i] = (button_internal_t){0};
        button_used[i] = false;
    }
    button_count = 0;
}

button_handle_t button_create(const button_config_t *config)
{
    if (button_count >= BUTTON_MAX_BUTTONS)
        return BUTTON_INVALID_HANDLE;

    for (int i = 0; i < BUTTON_MAX_BUTTONS; i++)
    {
        if (!button_used[i])
        {
            buttons[i].port = config->port;
            buttons[i].pin = config->pin;
            buttons[i].debounce_ms = (config->debounce_ms > 0) ? config->debounce_ms : 50;
            buttons[i].long_press_ms = (config->long_press_ms > 0) ? config->long_press_ms : 1000;
            buttons[i].active_low = config->active_low;

            GPIO_InitTypeDef gpio_cfg = {0};
            gpio_cfg.Pin = config->pin;
            gpio_cfg.Mode = GPIO_MODE_INPUT;
            gpio_cfg.Pull = config->pull_up ? GPIO_PULLUP : GPIO_PULLDOWN;
            HAL_GPIO_Init(config->port, &gpio_cfg);

            buttons[i].estado_filtrado = button_raw_read(&buttons[i]);
            buttons[i].estado_anterior = buttons[i].estado_filtrado;
            buttons[i].ultimo_cambio = millis();
            buttons[i].inicio_presion = 0;
            buttons[i].flag_presionado = false;
            buttons[i].flag_liberado = false;
            buttons[i].flag_long_press = false;
            buttons[i].long_press_triggered = false;

            button_used[i] = true;
            button_count++;
            return (button_handle_t)i;
        }
    }
    return BUTTON_INVALID_HANDLE;
}

void button_update_all(void)
{
    uint32_t ahora = millis();

    for (int i = 0; i < BUTTON_MAX_BUTTONS; i++)
    {
        if (!button_used[i])
            continue;

        button_internal_t *btn = &buttons[i];
        bool estado_raw = button_raw_read(btn);

        if (estado_raw != btn->estado_anterior)
        {
            btn->ultimo_cambio = ahora;
            btn->estado_anterior = estado_raw;
        }

        if ((ahora - btn->ultimo_cambio) >= btn->debounce_ms)
        {
            bool estado_anterior_filtrado = btn->estado_filtrado;
            btn->estado_filtrado = estado_raw;

            // Detectar presión (flanco de bajada)
            btn->flag_presionado = (!estado_anterior_filtrado && btn->estado_filtrado);

            // Detectar liberación (flanco de subida)
            btn->flag_liberado = (estado_anterior_filtrado && !btn->estado_filtrado);

            // Registrar inicio de presión
            if (btn->flag_presionado)
            {
                btn->inicio_presion = ahora;
                btn->long_press_triggered = false;
            }

            // Detectar presión larga
            if (btn->estado_filtrado && !btn->long_press_triggered)
            {
                if ((ahora - btn->inicio_presion) >= btn->long_press_ms)
                {
                    btn->flag_long_press = true;
                    btn->long_press_triggered = true;
                }
            }
        }
    }
}

bool button_pressed(button_handle_t handle)
{
    if (handle >= BUTTON_MAX_BUTTONS || !button_used[handle])
        return false;
    bool result = buttons[handle].flag_presionado;
    buttons[handle].flag_presionado = false;
    return result;
}

bool button_released(button_handle_t handle)
{
    if (handle >= BUTTON_MAX_BUTTONS || !button_used[handle])
        return false;
    bool result = buttons[handle].flag_liberado;
    buttons[handle].flag_liberado = false;
    return result;
}

bool button_is_pressed(button_handle_t handle)
{
    if (handle >= BUTTON_MAX_BUTTONS || !button_used[handle])
        return false;
    return buttons[handle].estado_filtrado;
}

bool button_long_pressed(button_handle_t handle)
{
    if (handle >= BUTTON_MAX_BUTTONS || !button_used[handle])
        return false;
    bool result = buttons[handle].flag_long_press;
    buttons[handle].flag_long_press = false;
    return result;
}

uint32_t button_pressed_duration(button_handle_t handle)
{
    if (handle >= BUTTON_MAX_BUTTONS || !button_used[handle])
        return 0;
    button_internal_t *btn = &buttons[handle];
    if (!btn->estado_filtrado)
        return 0;
    return millis() - btn->inicio_presion;
}