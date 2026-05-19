
/******************************************************************************
 * @author         : Rodrigo Calle Condori
 * @file           : main.h
 * @brief          : Este es una libreria para la configuracion de los
 *                   perifericos gpio contiene los pines de los Display de 7
 *                   segmentos, 4 botones y un buzzer, utilizan los puertos A y B
 */
#pragma once
#include "stm32f1xx_hal.h" // ← ESTE ES EL IMPORTANTE (incluye todo)
/*======Definicion de pines a usar===========*/
// Pines del Display de 7 segmentos usados en el PORT A
#define PIN_A1 GPIO_PIN_1
#define PIN_A2 GPIO_PIN_2
#define PIN_A3 GPIO_PIN_3
#define PIN_A4 GPIO_PIN_4
#define PIN_A5 GPIO_PIN_5
#define PIN_A6 GPIO_PIN_6
#define PIN_A7 GPIO_PIN_7
// Pines de on-off para activar display PORT A
#define DISPLAY_1 GPIO_PIN_12
#define DISPLAY_2 GPIO_PIN_11
#define DISPLAY_3 GPIO_PIN_10

/*======Definicion de pulsadores=========*/
// Pusadores conectados al PORTB
#define BUTTON_RESET GPIO_PIN_12
#define BUTTON_DOWN GPIO_PIN_13
#define BUTTON_UP GPIO_PIN_14
#define BUTTON_CONF_STR GPIO_PIN_15

/*======Definicion de Buzzer=============*/
// Buzzer conectado al PORTA
#define BUZZER_PIN GPIO_PIN_9

/*
 *  @brief :Inicializar pines Display, buzzer y pulsadores
 */
void hardware_init(void);