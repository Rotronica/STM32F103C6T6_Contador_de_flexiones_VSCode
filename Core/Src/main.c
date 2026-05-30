/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "board_init.h" //Libreria creado para la configuracion de pines del proyecto
#include "time_ticks.h"
#include "Display7seg.h"
#include "VL53L0X.h"
#include "buzzer.h"
#include "flexiones.h"
#include "button.h"
#include "menu.h"
#include "bateria.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//==Configuracion del estado para hacer flexiones=================
#define UMBRAL_ALTO 250
#define UMBRAL_BAJO 70
#define OBJETIVO_FLEXIONES 15
//================================================================

//=======Tiempos para avisar baja bateria por buzzer===============
#define TIEMPO_DE_PITIDO_LOW 500         // Tiempo del pitido
#define CICLOS_DE_PITIDOS_LOW 2          // Un pitido sonara 3 veces y luego se apaga
#define TIEMPO_DE_ADVERTENCIA_LOW 180000 // Cada 3 minutos aparecera el pitido
//================================================================

//=======Tiempos para avisar bateria critica con buzzer
#define TIEMPO_DE_PITIDO_WARNING 500        // Tiempo del pitido
#define CICLOS_DE_PITIDOS_WARNING 5         // Un pitido sonara 4 veces y luego se apaga
#define TIEMPO_DE_ADVERTENCIA_WARNING 60000 // Cada minuto aparecera el pitido
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static button_handle_t btn_start, btn_up, btn_down, btn_reset;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void crear_botones(void);      // Funcion creada para configurar cada boton
void config_bateria(void);     // Configuracion de las condiciones de la bateria
void monitorear_bateria(void); // Funcion para monitorear las baterias

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2); // Activa la interrupcion del TMR2
  hardware_init();               // inicializa los pines display,buzzer y button
  buzzer_init();
  flexiones_init();
  // Inicializar la medicion de baterias
  config_bateria();
  battery_status_t bat_status;
  Display7seg_init();

  // Inicializar botones
  button_init();
  crear_botones();
  // iniciar menu
  static menu_t menu;
  menu_init(&menu, btn_start, btn_up, btn_down, btn_reset, OBJETIVO_FLEXIONES, UMBRAL_BAJO, UMBRAL_ALTO);

  // Inicializar sensor
  static VL53L0X_t sensor;

  VL53L0X_Init(&sensor, &hi2c1, true);
  VL53L0X_SetHighSpeedMode(&sensor); // Modo rápido para flexiones
  VL53L0X_SetTimeout(&sensor, 30);   // Timeout de 30ms

  Display7seg_show_text("inicio");

  buzzer_start(500); // 500ms de pitido

  uint16_t distance = 0;
  uint16_t contador = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    Display7seg_refresh();       // Multiplexado del display
    button_update_all();         // Actualizar estado de botones
    buzzer_update();             // Actualizar estado del buzzer
    menu_update(&menu);          // Actualizar lógica del menú
    battery_update(&bat_status); // Actualiza el estado de la bateria
    monitorear_bateria();        // Monitorear siempre la bateria
    //=====================Modo espera======================
    if (menu_get_estado(&menu) == MODO_ESPERA)
    {
      menu_update_display(&menu);
    }
    //===================Modo conteo========================
    if (menu_get_estado(&menu) == MODO_CONTEO)
    {
      // Verificar reset solicitado por el menú
      // if (menu_cont_rst(&menu))
      //{
      // flexiones_cont_reset();
      //}
      // Iniciar medición cada 25ms (más rápido que el tiempo de medición)
      if (VL53L0X_StartMeasurementIfReady(&sensor, 25))
      {
        // Medición iniciada
      }

      if (VL53L0X_IsReady(&sensor))
      {
        if (!VL53L0X_TimeoutOccurred(&sensor))
        {
          distance = VL53L0X_GetDistance(&sensor);
          // Procesar distancia...
          contador = flexiones_actualizar(distance);
          Display7seg_show_number(contador);
        }
      }
    }
    //===========Estado de la bateria======================
    if (menu_get_estado(&menu) == ESTADO_BATERIA)
    {
      menu_update_display(&menu);
    }
    //===========Modo configuracion========================
    if (menu_get_estado(&menu) == MODO_CONFIGURACION)
    {
      menu_update_display(&menu);
    }
    //===========Opcion objetivos========================
    if (menu_get_estado(&menu) == OPCION_OBJETIVOS)
    {
      menu_update_display(&menu);
    }
    //===========Opcion umbral========================
    if (menu_get_estado(&menu) == OPCION_UMBRAL)
    {
      menu_update_display(&menu);
    }
    //===========Opcion modo medicion===============
    if (menu_get_estado(&menu) == OPCION_MEDICION)
    {
      // Iniciar medición cada 25ms (más rápido que el tiempo de medición)
      if (VL53L0X_StartMeasurementIfReady(&sensor, 25))
      {
        // Medición iniciada
      }

      if (VL53L0X_IsReady(&sensor))
      {
        if (!VL53L0X_TimeoutOccurred(&sensor))
        {
          menu_update_display(&menu);
          distance = VL53L0X_GetDistance(&sensor);
          // Display7seg_show_number(distance);
          menu_read(distance);
        }
      }
    }
    //===========Umbral alto========================
    if (menu_get_estado(&menu) == UMBRAL_UP)
    {
      menu_update_display(&menu);
    }
    //===========Umbral bajo========================
    if (menu_get_estado(&menu) == UMBRAL_DOWN)
    {
      menu_update_display(&menu);
    }
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void crear_botones(void)
{
  // Atributos del pulsador start/Configuracion
  button_config_t btn_start_config = {
      .pin = BUTTON_CONF_STR,
      .port = GPIOB,
      .pull_up = true,
      .active_low = true,
      .debounce_ms = 50,
      .long_press_ms = 1000,
  };
  btn_start = button_create(&btn_start_config);
  // Atributos del boton up
  button_config_t btn_arriba = {
      .pin = BUTTON_UP,
      .port = GPIOB,
      .pull_up = true,
      .active_low = true,
      .debounce_ms = 50,
      .long_press_ms = 1000,
  };
  btn_up = button_create(&btn_arriba);
  // Atributos del boton down
  button_config_t btn_abajo = {
      .pin = BUTTON_DOWN,
      .port = GPIOB,
      .pull_up = true,
      .active_low = true,
      .debounce_ms = 10,
      .long_press_ms = 1000,
  };
  btn_down = button_create(&btn_abajo);
  // Atributos del boton Reset/Back
  button_config_t btn_rst = {
      .pin = BUTTON_RESET,
      .port = GPIOB,
      .pull_up = true,
      .active_low = true,
      .debounce_ms = 10,
      .long_press_ms = 1000,
  };
  btn_reset = button_create(&btn_rst);
}
// Funcion para configurar la inicializacion de la medicion de la bateria
void config_bateria(void)
{
  battery_config_t cfg_battery = {
      .voltaje_max = 8.40f,        // 4.2V × 2 (totalmente cargada)
      .voltaje_min = 6.0f,         // 3.0V × 2 (totalmente vacía)
      .voltaje_advertencia = 6.9f, // Advertencia de batería baja
      .voltaje_critico = 6.6f,     // 6.6V (crítica)
      .divisor_tension = 2.55f,    // Ajustar según el divisor (ej: (10k+5.6k)/5.6k)
  };
  // En la inicialización
  battery_init(&cfg_battery);
}

void monitorear_bateria(void)
{
  static uint32_t last_alert_critical = 0;
  static uint32_t last_alert_low = 0;
  static bool primera_critical = true;
  static bool primera_low = true;
  uint32_t ahora = millis();

  // ========= Batería CRÍTICA (más urgente) =========
  if (battery_is_critical())
  {
    // PRIMERA ALERTA: inmediata
    if (primera_critical)
    {
      primera_critical = false;
      last_alert_critical = ahora;
      buzzer_alarm_start(TIEMPO_DE_PITIDO_WARNING,
                         CICLOS_DE_PITIDOS_WARNING,
                         TIEMPO_DE_ADVERTENCIA_WARNING);
    }
    // ALERTAS SIGUIENTES: cada TIEMPO_DE_ADVERTENCIA_WARNING
    else if (ahora - last_alert_critical >= TIEMPO_DE_ADVERTENCIA_WARNING)
    {
      last_alert_critical = ahora;
      buzzer_alarm_start(TIEMPO_DE_PITIDO_WARNING,
                         CICLOS_DE_PITIDOS_WARNING,
                         TIEMPO_DE_ADVERTENCIA_WARNING);
    }
  }
  // ========= Batería BAJA (menos urgente) =========
  else if (battery_is_low())
  {
    primera_critical = true; // Resetear para cuando vuelva a crítica

    // PRIMERA ALERTA: inmediata
    if (primera_low)
    {
      primera_low = false;
      last_alert_low = ahora;
      buzzer_alarm_start(TIEMPO_DE_PITIDO_LOW,
                         CICLOS_DE_PITIDOS_LOW,
                         TIEMPO_DE_ADVERTENCIA_LOW);
    }
    // ALERTAS SIGUIENTES: cada TIEMPO_DE_ADVERTENCIA_LOW
    else if (ahora - last_alert_low >= TIEMPO_DE_ADVERTENCIA_LOW)
    {
      last_alert_low = ahora;
      buzzer_alarm_start(TIEMPO_DE_PITIDO_LOW,
                         CICLOS_DE_PITIDOS_LOW,
                         TIEMPO_DE_ADVERTENCIA_LOW);
    }
  }
  // ========= Batería NORMAL =========
  else
  {
    // Resetear todo cuando la batería se recupera
    primera_critical = true;
    primera_low = true;
    last_alert_critical = 0;
    last_alert_low = 0;
    if (buzzer_alarm_is_active())
    {
      buzzer_alarm_stop();
    }
  }
}
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */