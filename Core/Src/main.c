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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define UMBRAL_UP 250
#define UMBRAL_DOWN 70
#define OBJETIVO_FLEXIONES 15
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
void crear_botones(void);
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
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim2); // Activa la interrupcion del TMR2
  hardware_init();               // inicializa los pines display,buzzer y button
  buzzer_init();
  flexiones_init();
  Display7seg_init();

  // Inicializar botones
  button_init();
  crear_botones();
  // iniciar menu
  static menu_t menu;
  menu_init(&menu, btn_start, btn_up, btn_down, btn_reset, OBJETIVO_FLEXIONES, UMBRAL_DOWN, UMBRAL_UP);

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
    Display7seg_refresh(); // Multiplexado del display
    button_update_all();   // Actualizar estado de botones
    buzzer_update();       // Actualizar estado del buzzer
    menu_update(&menu);    // Actualizar lógica del menú
    //=====================Modo espera======================
    if (menu_get_estado(&menu) == MODO_ESPERA)
    {
      menu_update_display(&menu);
    }
    //===================Modo conteo========================
    if (menu_get_estado(&menu) == MODO_CONTEO)
    {
      // Verificar reset solicitado por el menú
      if (menu_cont_rst(&menu))
      {
        flexiones_cont_reset();
      }
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
    //===========Modo configuracion========================
    if (menu_get_estado(&menu) == MODO_CONFIGURACION)
    {
      menu_update_display(&menu);
    }
    /* USER CODE END 3 */
  }
}
/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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