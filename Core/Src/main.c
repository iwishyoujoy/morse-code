/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char letters[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int morse[] = {22222, 12222, 11222, 11122, 11112, 11111, 21111, 22111, 22211, 22221, 12, 2111, 2121, 211, 1, 1121, 221, 1111, 11, 1222, 212, 1211, 22, 21, 222, 1221, 2212, 121, 111, 2, 112, 1112, 122, 2112, 2122, 2211};
_Bool isUseInterrupts = 0;

char buffer[8];
int bufferGetPosition = 0;
int bufferAddPosition = 0;

_Bool isPressed() { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_RESET; }

_Bool isNotPressed() { return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_15) == GPIO_PIN_SET; }

_Bool isBufferEmpty() { return (buffer[0] == 0 && buffer[1] == 0 && buffer[2] == 0 && buffer[3] == 0 && buffer[4] == 0 && buffer[5] == 0 && buffer[6] == 0 && buffer[7] == 0); }

_Bool isBufferFull() { return (buffer[0] != 0 && buffer[1] != 0  && buffer[2] != 0 && buffer[3] != 0 && buffer[4] != 0 && buffer[5] != 0 && buffer[6] != 0 && buffer[7] != 0); }

void turnOn() {
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
}

void turnOff() {
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}

void addToBuffer(char s) {
	buffer[bufferAddPosition] = s;
	bufferAddPosition++;
	if (bufferAddPosition > 7) {
		bufferAddPosition = 0;
	}
}

char getCharFromBuffer() {
	char toReturn = buffer[bufferGetPosition];
	buffer[bufferGetPosition] = 0;
	bufferGetPosition++;
	if (bufferGetPosition > 7) {
		bufferGetPosition = 0;
	}
	return toReturn;
}

int getMorseCode(char s) {
	int position = -1;
	for (int i = 0; i < sizeof(letters); i++) {
		if (s == letters[i]) {
			position = i;
			break;
		}
	}
	if (position >= 0) {
		return morse[position];
	} else {
		return position;
	}
}

char getCharByMorseCode(int s) {
	int position = -1;
	for (int i = 0; i < sizeof(morse); i++) {
		if (s == morse[i]) {
			position = i;
			break;
		}
	}
	if (position >= 0) {
		return letters[position];
	} else {
		return '-';
	}
}

int uart6_recieve_finished;
int uart6_transmit_ongoing;

static uint8_t uart6_buf;

HAL_StatusTypeDef uart6_start_recieve_char_it() {
	uart6_recieve_finished = 0;
	return HAL_UART_Receive_IT(&huart6, &uart6_buf, 1);
}

int uart6_try_get_received_char(uint8_t *buf) {
	if (uart6_recieve_finished) {
		*buf = uart6_buf;
		return 1;
	}

	return 0;
}

HAL_StatusTypeDef uart6_transmit_it (uint8_t *buf, int len) {
	while (uart6_transmit_ongoing);

	uart6_transmit_ongoing = 1;
	return HAL_UART_Transmit_IT(&huart6, buf, len);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART6) {
		uart6_recieve_finished = 1;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART6) {
		uart6_transmit_ongoing = 0;
	}
}

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
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  char recieved, current, toSend[3];
  int currentMorse = -1;
  int morseTen = 5;
  int btnMorse = 0;
  int btnCounter = 0;
  _Bool isLightOn = 0;
  _Bool isPause = 0;
  _Bool isBtnPressed = 0;
  _Bool isBtnPause = 0;
  uint32_t blinkStartTime, pauseStartTime, diffLO, diffPause, btnStart, btnDiff, btnPauseStart, btnPauseDiff;
  int blinkingMode;
  char err[] = "Unsupported symbol by button \n";
  char intOn[] = "Interrupts turned On \n";
  char intOff[] = "Interrupts turned Off \n";
  char ready[] = "Ready for work \n";

  HAL_UART_Transmit(&huart6, (uint8_t *) ready, sizeof(ready), 10);

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  if (!isUseInterrupts) {
		//No Interrupts
		if (HAL_OK == HAL_UART_Receive(&huart6, (uint8_t *) &recieved, 1, 1)) {
			char toret[3];
			toret[0] = recieved;
			toret[1] = '\n';
			toret[2] = '\r';
			HAL_UART_Transmit(&huart6, (uint8_t *) toret, 3, 10);

			if (recieved == '+') {
				isUseInterrupts = 1;
				HAL_UART_Transmit(&huart6, (uint8_t *) intOn, sizeof(intOn), 10);
				HAL_NVIC_EnableIRQ(USART6_IRQn);
				uart6_start_recieve_char_it();
				continue;
			}

			if (!isBufferFull()) {
				addToBuffer(recieved);
			}

		}

		if (!isLightOn && !isPause) {
			if (!isBufferEmpty() && currentMorse < 0) {
				current = getCharFromBuffer();
				currentMorse = getMorseCode(current);
			}

			if (currentMorse >= 0) {
				blinkingMode = currentMorse / pow(10, morseTen);
				currentMorse = currentMorse % (int)pow(10, morseTen);
				morseTen--;
				if (morseTen == -1) {
					morseTen = 5;
					currentMorse = -1;
				}

				if (blinkingMode > 0) {
					blinkStartTime = HAL_GetTick();
					turnOn();
					isLightOn = 1;
				}

			}
		}

		if (isLightOn) {
			diffLO = HAL_GetTick() - blinkStartTime;
			if (blinkingMode == 1) {
				if (diffLO >= 300) {
					turnOff();
					pauseStartTime = HAL_GetTick();
					isLightOn = 0;
					isPause = 1;
				}
			} else if (blinkingMode == 2) {
				if (diffLO >= 700) {
					turnOff();
					pauseStartTime = HAL_GetTick();
					isLightOn = 0;
					isPause = 1;
				}
			}
		}

		if (isPause) {
			diffPause = HAL_GetTick() - pauseStartTime;
			if (diffPause >= 300) {
				isPause = 0;
			}
		}

		if (!isBtnPressed) {
			if (isPressed()) {
				isBtnPressed = 1;
				isBtnPause = 0;
				btnStart = HAL_GetTick();
			}
		}

		if (isBtnPressed) {
			if (isNotPressed()) {
				isBtnPressed = 0;
				isBtnPause = 1;
				btnDiff = HAL_GetTick() - btnStart;
				btnPauseStart = HAL_GetTick();

				if (btnDiff >= 100) {
					if (btnDiff < 500) {
						btnMorse = btnMorse * 10 + 1;
						btnCounter++;
					} else {
						btnMorse = btnMorse * 10 + 2;
						btnCounter++;
					}
				}
			}
		}


		if (isBtnPause) {
			if (isNotPressed()) {
				btnPauseDiff = HAL_GetTick() - btnPauseStart;

				if (btnPauseDiff > 1000) {
					toSend[0] = getCharByMorseCode(btnMorse);
					if (toSend[0] == '-') {
						HAL_UART_Transmit(&huart6, (uint8_t *) err, sizeof(err), 10);
					} else {
						toSend[1] = '\n';
						toSend[2] = '\r';
						HAL_UART_Transmit(&huart6, (uint8_t *) toSend, 3, 10);

						if (!isBufferFull()) {
							addToBuffer(toSend[0]);
						}
					}
					isBtnPause = 0;
					btnMorse = 0;
					btnCounter = 0;
				}
			}
		}

	  } else {
		//With Interrupts
		if (uart6_try_get_received_char((uint8_t *) &recieved)) {
			uart6_start_recieve_char_it();

			char toret[3];
			toret[0] = recieved;
			toret[1] = '\n';
			toret[2] = '\r';
			uart6_transmit_it((uint8_t *) &toret, 3);
			if (recieved == '+') {
				isUseInterrupts = 0;
				HAL_UART_Abort_IT(&huart6);
				HAL_NVIC_DisableIRQ(USART6_IRQn);
				HAL_UART_Transmit(&huart6, (uint8_t *) intOff, sizeof(intOff), 10);
				continue;
			}

			if (!isBufferFull()) {
				addToBuffer(recieved);
			}
		}

		if (!isLightOn && !isPause) {
			if (!isBufferEmpty() && currentMorse < 0) {
				current = getCharFromBuffer();
				currentMorse = getMorseCode(current);
			}

			if (currentMorse >= 0) {
				blinkingMode = currentMorse / pow(10, morseTen);
				currentMorse = currentMorse % (int)pow(10, morseTen);
				morseTen--;
				if (morseTen == -1) {
					morseTen = 5;
					currentMorse = -1;
				}

				if (blinkingMode > 0) {
					blinkStartTime = HAL_GetTick();
					turnOn();
					isLightOn = 1;
				}

			}
		}

		if (isLightOn) {
			diffLO = HAL_GetTick() - blinkStartTime;
			if (blinkingMode == 1) {
				if (diffLO >= 300) {
					turnOff();
					pauseStartTime = HAL_GetTick();
					isLightOn = 0;
					isPause = 1;
				}
			} else if (blinkingMode == 2) {
				if (diffLO >= 700) {
					turnOff();
					pauseStartTime = HAL_GetTick();
					isLightOn = 0;
					isPause = 1;
				}
			}
		}

		if (isPause) {
			diffPause = HAL_GetTick() - pauseStartTime;
			if (diffPause >= 300) {
				isPause = 0;
			}
		}

		if (!isBtnPressed) {
			if (isPressed()) {
				isBtnPressed = 1;
				isBtnPause = 0;
				btnStart = HAL_GetTick();
			}
		}

		if (isBtnPressed) {
			if (isNotPressed()) {
				isBtnPressed = 0;
				isBtnPause = 1;
				btnDiff = HAL_GetTick() - btnStart;
				btnPauseStart = HAL_GetTick();

				if (btnDiff >= 100) {
					if (btnDiff < 500) {
						btnMorse = btnMorse * 10 + 1;
						btnCounter++;
					} else {
						btnMorse = btnMorse * 10 + 2;
						btnCounter++;
					}
				}
			}
		}

		if (isBtnPause) {
			if (isNotPressed()) {
				btnPauseDiff = HAL_GetTick() - btnPauseStart;

				if (btnPauseDiff > 1000) {
					toSend[0] = getCharByMorseCode(btnMorse);
					if (toSend[0] == '-') {
						uart6_transmit_it((uint8_t *) &err, sizeof(err));
					} else {
						toSend[1] = '\n';
						toSend[2] = '\r';
						uart6_transmit_it((uint8_t *) &toSend, sizeof(toSend));

						if (!isBufferFull()) {
							addToBuffer(toSend[0]);
						}
					}
					isBtnPause = 0;
					btnMorse = 0;
					btnCounter = 0;
				}
			}
		}
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PD13 PD14 PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
