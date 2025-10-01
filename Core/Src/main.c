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
#include "ili9341.h"
#include <time.h>
#include <stdlib.h>
#include <cJSON.h>
#include <string.h>
#include "XPT2046_touch.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define min(a,b) (((a)<(b))?(a):(b))
#define BUFFER_SIZE 1024
#define PORTRAIT 0
#define NOBACKCOLOR 0
#define NHATRANG 0
#define SAIGON 1
#define HANOI 2
#define TAMPERE 3
#define ARNHEM 4
#define SYDNEY 5
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */
//URLs to call API
const char* nhaTrangURL = "http://api.open-meteo.com/v1/forecast?latitude=12.2451&longitude=109.1943&current=temperature_2m,relative_humidity_2m,is_day&daily=weather_code&timezone=auto\r\n";
const char* saiGonURL = "http://api.open-meteo.com/v1/forecast?latitude=10.823&longitude=106.6296&current=temperature_2m,relative_humidity_2m,is_day&daily=weather_code&timezone=auto\r\n";
const char* haNoiURL = "http://api.open-meteo.com/v1/forecast?latitude=20.9714&longitude=105.7788&current=temperature_2m,relative_humidity_2m,is_day&daily=weather_code&timezone=auto\r\n";
const char* tampereURL = "http://api.open-meteo.com/v1/forecast?latitude=61.4991&longitude=23.7871&current=temperature_2m,relative_humidity_2m,is_day&daily=weather_code&timezone=auto\r\n";
const char* arnhemURL = "http://api.open-meteo.com/v1/forecast?latitude=51.98&longitude=5.9111&current=temperature_2m,relative_humidity_2m,is_day&daily=weather_code&timezone=auto\r\n";
const char* sydneyURL = "http://api.open-meteo.com/v1/forecast?latitude=-33.8678&longitude=151.2073&current=temperature_2m,relative_humidity_2m,is_day&daily=weather_code&timezone=auto\r\n";
//Variables for handling received UART data
uint8_t tempBuffer[1];
uint8_t rxBuffer[BUFFER_SIZE];
uint16_t rxIndex = 0;
uint8_t rxComplete = 0;

//Variables for switching between UIs
uint16_t xCoordinates = 0, yCoordinates = 0;
uint8_t menu = 1;

//Variables for weather data handling
int weather[7];
int currentTemperature = 0;
int currentHumidity = 0;
int currentCity = 0;
uint16_t weatherIndex = 0;
uint16_t dateIndex = 0;
uint8_t processComplete = 0;
uint8_t isDay = 0;
char* date[7];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_FSMC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void drawMenuIcon();
void drawMenu();
void drawBufferScreen();
void checkCoordinates();
void drawWeather(uint16_t xPosition, uint16_t yPosition, int weatherCode);
void drawInterface();
void reformatDate();
void sendAPIURL(uint16_t chooseCity);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void processWeatherData(const char *jsonData);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
void resetBuffer();
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
  MX_FSMC_Init();
  MX_USART1_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */
  LCD_BL_ON();
  lcdInit();
  lcdSetOrientation(PORTRAIT);
  HAL_UART_Receive_IT(&huart1, (uint8_t*)tempBuffer, 1);
  drawMenu();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if(rxComplete)
	  {
		  rxComplete = 0;
		  rxIndex = 0;
		  processWeatherData((const char*) rxBuffer);
		  if (processComplete)
		  {
			  processComplete = 0;
			  weatherIndex = 0;
			  dateIndex = 0;
			  drawInterface();
		  }
		  resetBuffer();
		  HAL_UART_Receive_IT(&huart1, (uint8_t*)tempBuffer, 1);
	  }
	  HAL_Delay(1000);
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
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(T_CS_GPIO_Port, T_CS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : T_IRQ_Pin */
  GPIO_InitStruct.Pin = T_IRQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(T_IRQ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_BL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : T_CS_Pin */
  GPIO_InitStruct.Pin = T_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(T_CS_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* FSMC initialization function */
static void MX_FSMC_Init(void)
{

  /* USER CODE BEGIN FSMC_Init 0 */

  /* USER CODE END FSMC_Init 0 */

  FSMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FSMC_Init 1 */

  /* USER CODE END FSMC_Init 1 */

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FSMC_NORSRAM_DEVICE;
  hsram1.Extended = FSMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FSMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FSMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FSMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FSMC_NORSRAM_MEM_BUS_WIDTH_16;
  hsram1.Init.BurstAccessMode = FSMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FSMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WrapMode = FSMC_WRAP_MODE_DISABLE;
  hsram1.Init.WaitSignalActive = FSMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FSMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FSMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FSMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FSMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FSMC_WRITE_BURST_DISABLE;
  hsram1.Init.PageSize = FSMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 1;
  Timing.AddressHoldTime = 15;
  Timing.DataSetupTime = 5;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FSMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FSMC_Init 2 */

  /* USER CODE END FSMC_Init 2 */
}

/* USER CODE BEGIN 4 */
void drawMenuIcon()
{
	lcdFillRoundRect(0, 0, 35, 23, 8, COLOR_WHITE);
	lcdFillRoundRect(3, 4, 29, 3, 2, COLOR_BLACK);
	lcdFillRoundRect(3, 10, 29, 3, 2, COLOR_BLACK);
	lcdFillRoundRect(3, 16, 29, 3, 2, COLOR_BLACK);
}

void drawMenu()
{
	lcdSetTextFont(&Font16);
	lcdSetTextColor(COLOR_WHITE, COLOR_BLACK);
	lcdFillRGB(COLOR_BLACK);
	menu = 1;
	drawAlignedText("Choose a location", 0, 240, 16, NOBACKCOLOR);

	lcdSetTextColor(COLOR_BLACK, COLOR_BLACK);
	lcdFillRoundRect(0, 32, 240, 40, 10, COLOR_WHITE);
	drawAlignedText("Sai Gon", 45, 240, 16, NOBACKCOLOR);

	lcdFillRoundRect(0, 80, 240, 40, 10, COLOR_WHITE);
	drawAlignedText("Nha Trang", 93, 240, 16, NOBACKCOLOR);

	lcdFillRoundRect(0, 128, 240, 40, 10, COLOR_WHITE);
	drawAlignedText("Ha Noi", 141, 240, 16, NOBACKCOLOR);

	lcdFillRoundRect(0, 176, 240, 40, 10, COLOR_WHITE);
	drawAlignedText("Tampere", 189, 240, 16, NOBACKCOLOR);

	lcdFillRoundRect(0, 224, 240, 40, 10, COLOR_WHITE);
	drawAlignedText("Arnhem", 237, 240, 16, NOBACKCOLOR);

	lcdFillRoundRect(0, 272, 240, 40, 10, COLOR_WHITE);
	drawAlignedText("Sydney", 285, 240, 16, NOBACKCOLOR);
}

void drawBufferScreen()
{
	menu = 0;
	lcdFillRGB(COLOR_BLACK);
	drawMenuIcon();
	lcdSetTextColor(COLOR_WHITE, COLOR_BLACK);
	drawAlignedText("Fetching weather data", 160, 240, 16, NOBACKCOLOR);
}

void checkCoordinates()
{
	if ((yCoordinates <= 30 && xCoordinates <= 30) && menu != 1)
	{
		drawMenu();
	}
	if ((yCoordinates >= 32 && yCoordinates <= 72) && menu == 1)
	{
		drawBufferScreen();
		sendAPIURL(SAIGON);
		currentCity = 1;
	}
	if ((yCoordinates >= 80 && yCoordinates <= 120) && menu == 1)
	{
		drawBufferScreen();
		sendAPIURL(NHATRANG);
		currentCity = 0;
	}
	if ((yCoordinates >= 128 && yCoordinates <= 168) && menu == 1)
	{
		drawBufferScreen();
		sendAPIURL(HANOI);
		currentCity = 2;
	}
	if ((yCoordinates >= 176 && yCoordinates <= 216) && menu == 1)
	{
		drawBufferScreen();
		sendAPIURL(TAMPERE);
		currentCity = 3;
	}
	if ((yCoordinates >= 224 && yCoordinates <= 264) && menu == 1)
	{
		drawBufferScreen();
		sendAPIURL(ARNHEM);
		currentCity = 4;
	}
	if ((yCoordinates >= 272 && yCoordinates <= 312) && menu == 1)
	{
		drawBufferScreen();
		sendAPIURL(SYDNEY);
		currentCity = 5;
	}
}

void drawWeather(uint16_t xPosition, uint16_t yPosition, int weatherCode)
{
	if (weatherCode == 0)
	{
		drawClearDay(xPosition, yPosition);
	}
	else if (weatherCode >= 1 && weatherCode <= 3)
	{
		drawCloudyDay(xPosition, yPosition);
	}
	else if ((weatherCode >= 51 && weatherCode <= 67) || (weatherCode >= 80 && weatherCode <= 82))
	{
		drawRainyDay(xPosition, yPosition);
	}
	else if ((weatherCode >= 71 && weatherCode <= 77) || (weatherCode == 85 || weatherCode == 86))
	{
		drawSnowyDay(xPosition, yPosition);
	}
	else if (weatherCode >= 95 && weatherCode <= 99)
	{
		drawStormyDay(xPosition, yPosition);
	}
	else if (weatherCode == 45 || weatherCode == 48)
	{
		drawFoggyDay(xPosition, yPosition);
	}
}

void drawInterface()
{
	menu = 0;
	uint16_t color;
	if (isDay == 0)
	{
		lcdDrawImage(0, 0, &imageNight);
		color = COLOR_NAVY;
		lcdSetTextColor(COLOR_WHITE, COLOR_BLACK);
	}
	else
	{
		lcdDrawImage(0, 0, &imageDay);
		color = COLOR_CYAN;
		lcdSetTextColor(COLOR_BLACK, COLOR_BLACK);
	}
	drawMenuIcon();

	for (int y = 160; y < 320; y++)
	{
		for (int x = 0; x < 240; x++)
		{
		    lcdDrawPixel(x, y, color);
		}
	}

	lcdSetTextFont(&Font12);

	reformatDate();

	drawWeather(40, 190, weather[0]);
	lcdSetCursor(25, 160);
	lcdPrintfNoBackColor(date[0]);

	drawWeather(90, 190, weather[1]);
	lcdSetCursor(75, 160);
	lcdPrintfNoBackColor(date[1]);

	drawWeather(150, 190, weather[2]);
	lcdSetCursor(135, 160);
	lcdPrintfNoBackColor(date[2]);

	drawWeather(200, 190, weather[3]);
	lcdSetCursor(185, 160);
	lcdPrintfNoBackColor(date[3]);

	drawWeather(65, 270, weather[4]);
	lcdSetCursor(50, 240);
	lcdPrintfNoBackColor(date[4]);

	drawWeather(120, 270, weather[5]);
	lcdSetCursor(105, 240);
	lcdPrintfNoBackColor(date[5]);

	drawWeather(175, 270, weather[6]);
	lcdSetCursor(160, 240);
	lcdPrintfNoBackColor(date[6]);

	lcdSetCursor(80, 110);
	lcdPrintfNoBackColor("Humidity: %d%%", currentHumidity);
	lcdSetTextFont(&Font20);
	lcdSetCursor(110, 90);
	lcdPrintfNoBackColor("%d", currentTemperature);

	switch (currentCity)
	{
	case NHATRANG:
		drawAlignedText("Nha Trang", 10, 240, 16, NOBACKCOLOR);
	case SAIGON:
		drawAlignedText("Sai Gon", 10, 240, 16, NOBACKCOLOR);
	case HANOI:
		drawAlignedText("Ha Noi", 10, 240, 16, NOBACKCOLOR);
	case TAMPERE:
		drawAlignedText("Tampere", 10, 240, 16, NOBACKCOLOR);
	case ARNHEM:
		drawAlignedText("Arnhem", 10, 240, 16, NOBACKCOLOR);
	case SYDNEY:
		drawAlignedText("Sydney", 10, 240, 16, NOBACKCOLOR);
	}
}

void reformatDate()
{
	char temp[6];
	for (int i = 0; i < 7; i++)
	{
	    if (date[i])
	    {
	        strncpy(temp, &date[i][8], 2);
	        temp[2] = '-';
	        strncpy(&temp[3], &date[i][5], 2);
	        temp[5] = '\0';
	        strcpy(date[i], temp);
	    }
	}
}

void sendAPIURL(uint16_t chooseCity)
{
    switch(chooseCity)
    {
    case NHATRANG:
    	HAL_UART_Transmit(&huart1, (uint8_t*)nhaTrangURL, strlen(nhaTrangURL), HAL_MAX_DELAY);
    	break;
    case SAIGON:
    	HAL_UART_Transmit(&huart1, (uint8_t*)saiGonURL, strlen(saiGonURL), HAL_MAX_DELAY);
    	break;
    case HANOI:
    	HAL_UART_Transmit(&huart1, (uint8_t*)haNoiURL, strlen(haNoiURL), HAL_MAX_DELAY);
    	break;
    case TAMPERE:
    	HAL_UART_Transmit(&huart1, (uint8_t*)tampereURL, strlen(tampereURL), HAL_MAX_DELAY);
    	break;
    case ARNHEM:
    	HAL_UART_Transmit(&huart1, (uint8_t*)arnhemURL, strlen(arnhemURL), HAL_MAX_DELAY);
    	break;
    case SYDNEY:
    	HAL_UART_Transmit(&huart1, (uint8_t*)sydneyURL, strlen(sydneyURL), HAL_MAX_DELAY);
    	break;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (rxIndex < BUFFER_SIZE - 1)
	{
		if (tempBuffer[0] == '!')
		{
			rxComplete = 1;
			return;
		}
		else if (tempBuffer[0] == '?')
		{
			lcdPrintf("Error fetching data\n");
			lcdPrintf("Go back to menu!\n");
			rxIndex = 0;
			resetBuffer();
			HAL_UART_Receive_IT(&huart1, (uint8_t*)tempBuffer, 1);
			return;
		}
		rxBuffer[rxIndex++] = tempBuffer[0];
	}
	else
	{
	    rxIndex = 0;
	}
	HAL_UART_Receive_IT(&huart1, (uint8_t*)tempBuffer, 1);
}

void processWeatherData(const char *jsonData)
{
    cJSON *root = cJSON_Parse(jsonData);
    if (root == NULL)
    {
        lcdPrintf("Error parsing JSON\n");
        lcdPrintf("Go back to menu!\n");
        cJSON_Delete(root);
        return;
    }

    cJSON *current = cJSON_GetObjectItem(root, "current");
    if (current)
    {
        cJSON *temperature = cJSON_GetObjectItem(current, "temperature_2m");
        cJSON *humidity = cJSON_GetObjectItem(current, "relative_humidity_2m");
        cJSON *dayOrNight = cJSON_GetObjectItem(current, "is_day");
        if (temperature)
        {
        	currentTemperature = temperature->valueint;
        }
        if (humidity)
        {
        	currentHumidity = humidity->valueint;
        }
        if (dayOrNight)
        {
        	isDay = dayOrNight->valueint;
        }
    }

    cJSON *daily = cJSON_GetObjectItem(root, "daily");
    if (daily)
    {
    	cJSON *weatherCodes = cJSON_GetObjectItem(daily, "weather_code");
    	cJSON *dateCodes = cJSON_GetObjectItem(daily, "time");
    	if (dateCodes && cJSON_IsArray(dateCodes))
    	{
    		for (int i = 0; i < 7; i++)
    	    {
    	        cJSON *dates = cJSON_GetArrayItem(dateCodes, i);
    	        if (dates)
    	        {
    	        	date[dateIndex++] = strdup(dates->valuestring);
    	        }
    	    }
    	}
        if (weatherCodes && cJSON_IsArray(weatherCodes))
        {
            for (int i = 0; i < 7; i++)
            {
                cJSON *code = cJSON_GetArrayItem(weatherCodes, i);
                if (code)
                {
                    weather[weatherIndex++] = code->valueint;
                }
            }
        }
    }
    processComplete = 1;
    cJSON_Delete(root);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == T_IRQ_Pin)
	{
		if(XPT2046_TouchPressed())
		{
			if(XPT2046_TouchGetCoordinates(&xCoordinates, &yCoordinates))
			{
				xCoordinates = 240 - xCoordinates;
				yCoordinates = 320 - yCoordinates;
				checkCoordinates();
			}
		}
	}
}

void resetBuffer()
{
	for (int i = 0; i < sizeof(rxBuffer); i++)
	{
		rxBuffer[i] = 0;
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
