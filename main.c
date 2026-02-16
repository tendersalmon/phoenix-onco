/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal_uart.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ssd1963_fsmc.h"
#include "ugui.h"
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
UART_HandleTypeDef huart2;

SRAM_HandleTypeDef hsram1;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FMC_Init(void);
static void MX_USART2_UART_Init(void);
void StartDefaultTask(void *argument);
void Start_menu (void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define TFT_W 480
#define TFT_H 272
#define BLACK 0x0000

static void Draw3BoxesHorizontal(void)
{
    const uint16_t marginX = 18;   // отступ слева/справа
    const uint16_t gapX    = 18;   // зазор между рамками
    const uint16_t y       = 130;  // позиция по Y (под заголовок)
    const uint16_t boxH    = 110;  // высота рамок
    const uint8_t  border  = 8;    // толщина рамки (как на картинке)

    uint16_t boxW = (TFT_W - 2*marginX - 2*gapX) / 3;

    uint16_t x1 = marginX;
    uint16_t x2 = marginX + boxW + gapX;
    uint16_t x3 = marginX + 2*(boxW + gapX);

    TFT_Draw_Rectangle(x1, y, boxW, boxH, border, BLACK);
    TFT_Draw_Rectangle(x2, y, boxW, boxH, border, BLACK);
    TFT_Draw_Rectangle(x3, y, boxW, boxH, border, BLACK);
}
static uint8_t TXT_OFF[] = {0xC2,0xDB,0xCA,0xCB,0x00}; // "ВЫКЛ" (CP1251)
static uint8_t TXT_MENU[]     = {0xCC,0xE5,0xED,0xFE,0x00}; // "Меню"
static uint8_t TXT_CURTEMP[]  = {
    0xD2,0xE5,0xEA,0xF3,0xF9,0xE0,0xFF,0x20,              // "Текущая "
    0xF2,0xE5,0xEC,0xEF,0xE5,0xF0,0xE0,0xF2,0xF3,0xF0,0xE0,0x00 // "температура"
}; // "Текущая температура"
static uint8_t TXT_DATA_OK[]  = {0xE4,0xE0,0xED,0xED,0xFB,0xE5,0x20,0xCE,0xCA,0x21,0x00}; // "данные ОК!"
static uint8_t TXT_DATA_ERR[] = {0xEE,0xF8,0xE8,0xE1,0xEA,0xE0,0x20,0xE4,0xE0,0xED,0xED,0xFB,0xF5,0x21,0x00}; // "ошибка данных!"

/* USER CODE END 0 */
int menu=0,ch=0,temperature1, temperature2,transmit, ta_state;
uint8_t Uart;
uint8_t tx_temp[3], calc_temperature[3];

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
  MX_FMC_Init();
  MX_USART2_UART_Init();

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}





/*
void init_display() {
	UG_GUI gui;
    UG_Init(&gui, SSD1963_draw_pixel, 480, 272);
    UG_FontSelect(&FONT_8X12_CYRILLIC);
    UG_PutString(10, 10, "Привет мир!");
}
*/





void Start_menu (void)
{
	char temp[4];
	if (menu==0)
	{
		uint8_t text[] = {
		    0xD2, 0xD0, 0xC5, 0xC1, 0xD3, 0xC5, 0xCC, 0xC0, 0xDF, 0x20,  // "Требуемая"
		    0xD2, 0xC5, 0xCC, 0xCF, 0xC5, 0xD0, 0xC0, 0xD2, 0xD3, 0xD0, 0xC0,0x00  // "температура"
		};

		TFT_Draw_String(100, 35,BLACK, WHITE, (uint8_t*)font_courier, text, 2);
		TFT_Draw_String(10, 70,WHITE, BLACK, (const uint8_t*)font_courier, "1:", 3);
			 	if (temperature1==0)
								{
									TFT_Draw_String(60, 70,WHITE, BLACK, (const uint8_t*)font_courier, "(char*)TXT_OFF", 3);
								}
								else
								{

									temp[0]=(temperature1/100)+0x30;//сотни
									temp[1]=((temperature1%100)/10)+0x30;//десятки
									temp[2]=((temperature1%100)%10)+0x30;//единицы
									temp[3]='\0';
									TFT_Draw_String(60, 70, WHITE, BLACK, (const uint8_t*)font_courier, temp, 3);
								}
				TFT_Draw_String(170, 70,WHITE, BLACK, (const uint8_t*)font_courier, "2:", 3);
				if (temperature2==0)
								{
									TFT_Draw_String(220, 70,WHITE, BLACK, (const uint8_t*)font_courier, "OFF", 3);
								}
								else
								{
									temp[0]=(temperature2/100)+0x30;//сотни
									temp[1]=((temperature2%100)/10)+0x30;//десятки
									temp[2]=((temperature2%100)%10)+0x30;//единицы
									temp[3]='\0';
									TFT_Draw_String(220, 70, WHITE, BLACK, (const uint8_t*)font_courier, temp, 3);
								}
				TFT_Draw_String(330, 70,WHITE, BLACK, (const uint8_t*)font_courier, "TA:", 3);
				if (ta_state==0)
								{
									TFT_Draw_String(400, 70,WHITE, BLACK, (const uint8_t*)font_courier, "OFF", 3);
								}
								else
								{
									temp[0]=(ta_state/100)+0x30;//сотни
									temp[1]=((ta_state%100)/10)+0x30;//десятки
									temp[2]=((ta_state%100)%10)+0x30;//единицы
									temp[3]='\0';
									TFT_Draw_String(400, 70, WHITE, BLACK, (const uint8_t*)font_courier, temp, 3);
								}
				TFT_Draw_String(40, 240,WHITE, BLACK, (const uint8_t*)font_courier, "(char*)TXT_MENU", 2);
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 15;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 255;
  Timing.BusTurnAroundDuration = 15;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, DIP_EN_Pin|BL_EN_Pin|DP_RESET_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : DIP_EN_Pin BL_EN_Pin DP_RESET_Pin */
  GPIO_InitStruct.Pin = DIP_EN_Pin|BL_EN_Pin|DP_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
int flag=0;
char ch1[4],ch2[4], ch3[4];
  /* USER CODE BEGIN 5 */
	Init_SSD1963();
	HAL_GPIO_WritePin(BL_EN_GPIO_Port, BL_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(DIP_EN_GPIO_Port, DIP_EN_Pin, GPIO_PIN_SET);
	TFT_Clear_Screen(WHITE);
	//init_display();
	//Start_menu();
//	TFT_fill_screen(100,140, RED);
	for(int i=0;i<40000;i++){
	}
	uint8_t initial_screen_text[] = {0xC0,0xEA,0xF2,0xE8,0xE2,0xED,0xFB,0xE5,0x20,0xEC,0xEE,0xE4,0xF3,0xEB,0xE8,0x00};
	TFT_Draw_String(1, 140, BLACK, WHITE, (uint8_t*)font_courier, initial_screen_text, 3);


	//TFT_Draw_Rectangle(100, 40, 80, 70, 5, BLACK);
	//TFT_Draw_Rectangle(60, 40, 80, 70, 5, BLACK);
	//TFT_Draw_Rectangle(60, 40, 80, 70, 5, BLACK);
	Draw3BoxesHorizontal();
	//TFT_Draw_String_UTF8(200,140, BLACK, WHITE, font_courier, "Здравствуй", 3);
	//TFT_Draw_String(200, 140, WHITE, BLACK, (uint8_t*)font_courier, "HELLO", 3);
	//TFT_Clear_Screen(WHITE);
	//TFT_Draw_Cyrillic_String(200, 140, WHITE, BLACK, font_courier, "Привет", 1);
	//TFT_Draw_Char(200, 140,WHITE, BLACK, (const uint8_t*)font_courier, 0xC0, 1); // 'А'
	//TFT_Draw_Char(208, 140,WHITE, BLACK, (const uint8_t*)font_courier, 0xCF, 1); // 'П'

	//TFT_Draw_Char(200, 140, BLACK, WHITE, (const uint8_t*)font_courier, 0xCF, 3); // П
	//TFT_Draw_Char(230, 140, BLACK, WHITE, (const uint8_t*)font_courier, 0xD0, 3); // р
	//TFT_Draw_Char(260, 140, BLACK, WHITE, (const uint8_t*)font_courier, 0xC8, 3); // и
	//TFT_Draw_Char(290, 140, BLACK, WHITE, (const uint8_t*)font_courier, 0xC2, 3); // в
	//TFT_Draw_Char(320, 140, BLACK, WHITE, (const uint8_t*)font_courier, 0xC5, 3); // е
	//TFT_Draw_Char(350, 140, BLACK, WHITE, (const uint8_t*)font_courier, 0xD2, 3); // т



	/* Infinite loop */
	for(;;)
	{
		if (transmit==1)
		{
		 tx_temp[0] = temperature1;
		 tx_temp[1] = temperature2;
		 tx_temp[2] = ta_state;
		 HAL_UART_Transmit (&huart2, tx_temp, sizeof(tx_temp), 100);
		 while (flag==0)
		 {
			 flag = __HAL_UART_GET_FLAG (&huart2, UART_FLAG_RXNE);
		 }
		flag=0;
		HAL_UART_Receive (&huart2, &Uart, 1, 100);
		if (Uart!=0)
		{
		TFT_Draw_String(200, 200,WHITE, BLACK, (const uint8_t*)font_courier, "(char*)TXT_DATA_OK", 1);
		}
		else
		{
			TFT_Draw_String(200, 200,WHITE, BLACK, (const uint8_t*)font_courier, "(char*)TXT_DATA_ERR", 1);
		}
		transmit=0;
	}

	flag = __HAL_UART_GET_FLAG (&huart2, UART_FLAG_RXNE);
	if (flag != 0)
	{
		HAL_UART_Receive (&huart2, calc_temperature, 3, 100);

		ch1[1]=calc_temperature[0];
		ch1[0]=(ch1[1]/100)+0x30;//сотни
		ch1[2]=((ch1[1]%100)%10)+0x30;//единицы
		ch1[1]=((ch1[1]%100)/10)+0x30;//десятки
		ch1[3]='\0';

		ch2[1]=calc_temperature[1];
		ch2[0]=(ch2[1]/100)+0x30;//сотни
		ch2[2]=((ch2[1]%100)%10)+0x30;//единицы
		ch2[1]=((ch2[1]%100)/10)+0x30;//десятки
		ch2[3]='\0';

		ch3[1]=calc_temperature[2];
		ch3[0]=(ch3[1]/100)+0x30;//сотни
		ch3[2]=((ch3[1]%100)%10)+0x30;//единицы
		ch3[1]=((ch3[1]%100)/10)+0x30;//десятки
		ch3[3]='\0';


		if (menu==0)
		{
			TFT_Draw_String(100, 115,WHITE, BLUE, (const uint8_t*)font_courier, "(char*)TXT_CURTEMP", 2);
			TFT_Draw_String(10, 150,WHITE, BLACK, (const uint8_t*)font_courier, "1:", 3);
			TFT_Draw_String(60, 150, WHITE, BLACK, (const uint8_t*)font_courier, ch1, 3);
			TFT_Draw_String(170, 150,WHITE, BLACK, (const uint8_t*)font_courier, "2:", 3);
			TFT_Draw_String(220, 150, WHITE, BLACK, (const uint8_t*)font_courier, ch2, 3);
			TFT_Draw_String(320, 150,WHITE, BLACK, (const uint8_t*)font_courier, "TA:", 3);
			TFT_Draw_String(400, 150, WHITE, BLACK, (const uint8_t*)font_courier, ch3, 3);
		}
		flag = 0;

	}
	}
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM11 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM11) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
