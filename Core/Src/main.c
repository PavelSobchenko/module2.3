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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
	GPIO_TypeDef *port;
	uint8_t increaseLvl;
	const uint16_t pin;
	uint32_t lastChangeTimestampMs;
} Led;

typedef struct Button Button;
typedef void (*ButtonHandler)(Button *btn);

struct Button {
	GPIO_TypeDef *port;
	const uint16_t pin;
	const ButtonHandler onPress;
	bool pressed;
	bool lastRaw;
	uint32_t lastChangeMS;
	Led *const *const leds;
	const uint8_t ledsCount;
};

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */

static void button_update(Button *b);
static void on_increase_press(Button *b);
static void on_stop_press(Button *b);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const uint8_t INTERVAL_MS = 100;
const uint8_t DEBOUNCE_MS = 30;
const uint8_t MAX_INCREASE_LVL = 10;
const uint16_t STOP_INTERVAL_MS = 2000;

const uint8_t LED_COUNT = 3;
static Led leds[] = {
		{ .pin =  RED_LED_1_Pin, .port = RED_LED_1_GPIO_Port, .increaseLvl = 2 },
		{ .pin =  GREEN_LED_Pin, .port = GREEN_LED_GPIO_Port, .increaseLvl = 5 },
		{ .pin =  RED_LED_2_Pin, .port = RED_LED_2_GPIO_Port, .increaseLvl = 10 },
};

static Led *const increase_leds[] = { &leds[0] };
static Led *const all_leds[] = { &leds[0], &leds[1], &leds[2] };

const uint8_t BUTTONS_COUNT = 2;
static Button buttons[] = {
		{
				.pin = INCREASE_BTN_Pin,
				.port = INCREASE_BTN_GPIO_Port,
				.onPress = on_increase_press,
				.leds = increase_leds,
				.ledsCount = 1
		},
		{
				.pin = STOP_BTN_Pin,
				.port = STOP_BTN_GPIO_Port,
				.onPress = on_stop_press,
				.leds = all_leds,
				.ledsCount = 3,
		},
};

bool allStoped = false;
uint32_t allStopedAtMs = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	// set all leds off
	for(size_t i = 0; i < LED_COUNT; i ++) {
		HAL_GPIO_WritePin(leds[i].port, leds[i].pin, GPIO_PIN_RESET);
	}
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  uint32_t now = HAL_GetTick();

	  if (allStoped && now - allStopedAtMs >= STOP_INTERVAL_MS) {
		  allStoped = false;
	  } else if (!allStoped) {
		  for(size_t i = 0; i < LED_COUNT; i ++) {
			  if (now - leds[i].lastChangeTimestampMs >= INTERVAL_MS * leds[i].increaseLvl) {
				  leds[i].lastChangeTimestampMs = now;
				  HAL_GPIO_TogglePin(leds[i].port, leds[i].pin);
			  }
		  }
	  }

	  for (size_t i = 0; i < BUTTONS_COUNT; i++) {
		  button_update(&buttons[i]);
	  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RED_LED_1_Pin|GREEN_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(RED_LED_2_GPIO_Port, RED_LED_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RED_LED_1_Pin GREEN_LED_Pin */
  GPIO_InitStruct.Pin = RED_LED_1_Pin|GREEN_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : INCREASE_BTN_Pin STOP_BTN_Pin */
  GPIO_InitStruct.Pin = INCREASE_BTN_Pin|STOP_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : RED_LED_2_Pin */
  GPIO_InitStruct.Pin = RED_LED_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(RED_LED_2_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

static void button_update(Button *b) {
	bool raw = HAL_GPIO_ReadPin(b->port, b->pin);
	uint32_t now = HAL_GetTick();

	if (raw != b->lastRaw) {
		b->lastRaw = raw;
		b->lastChangeMS = now;
		return;
	}

	if (raw != b->pressed && now - b->lastChangeMS >= DEBOUNCE_MS) {
		b->pressed = raw;
		if (b->pressed) {
			b->onPress(b);
		}
	}
}

static void on_increase_press(Button *b) {
	for (size_t i = 0; i < b->ledsCount; i++) {
		uint8_t currntLvl = b->leds[i]->increaseLvl;
		b->leds[i]->increaseLvl = (currntLvl + 1) % MAX_INCREASE_LVL;
	}
}

static void on_stop_press(Button *b) {
	allStoped = true;
	allStopedAtMs = HAL_GetTick();

	for (size_t i = 0; i < b->ledsCount; i++) {
		HAL_GPIO_WritePin(b->leds[i]->port, b->leds[i]->pin, GPIO_PIN_RESET);
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
