/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ssd1306.h"

// https://github.com/nayuki/QR-Code-generator
// https://www.nayuki.io/page/qr-code-generator-library
#include "qrcodegen.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LIN_SEND_PERIOD 50
#define TX_BUFFER_SIZE 6
#define RX_BUFFER_SIZE_A 12
#define RX_BUFFER_SIZE_B 12
#define RX_BUFFER_SIZE_C 12
#define RX_BUFFER_SIZE_D 12

#define TX_LIN_MSG_ID 0x01 // backlight brightness
#define RX_LIN_MSG_ID_A 0x00 // buttons and knobs (repeated?)
#define RX_LIN_MSG_ID_B 0x02 // buttons and knobs
#define RX_LIN_MSG_ID_C 0x04 // counters
#define RX_LIN_MSG_ID_D 0x05 // diagnostics(?)
#define NUMBER_OF_RX_MSGS 4

#define ENC_KNOB_CNT_MIN 0
#define ENC_KNOB_CNT_MAX (255 * 4)

#define UART_SEND_PERIOD 500
#define OLED_UPDATE_PERIOD 250

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */
uint8_t txData[TX_BUFFER_SIZE];
uint8_t rxDataA[RX_BUFFER_SIZE_A];
uint8_t rxDataB[RX_BUFFER_SIZE_B];
uint8_t rxDataBbis[RX_BUFFER_SIZE_B];
uint8_t rxDataC[RX_BUFFER_SIZE_C];
uint8_t rxDataD[RX_BUFFER_SIZE_D];

uint32_t LinSendSoftTimer;

volatile uint8_t lin_data_received_flag = 0;
uint8_t rx_msg_index = 0;

extern DMA_HandleTypeDef hdma_usart1_rx;

char lcd_line[64];
uint8_t backlight_intensity = 0xFF;

uint32_t UartMsgSoftTimer;
uint32_t OledSoftTimer;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// https://controllerstech.com/stm32-uart-8-lin-protocol-part-1/
// https://controllerstech.com/stm32-uart-9-lin-protocol-part-2/
// https://controllerstech.com/stm32-uart-10-lin-protocol-part-3/
uint8_t Pid_Calc(uint8_t ID);
uint8_t Checksum_Calc(uint8_t PID, uint8_t *data, uint8_t size);

static void showPartDatasheet(void);
static void printQr(const uint8_t qrcode[]);
static void displayQr(const uint8_t qrcode[]);

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
	MX_DMA_Init();
	MX_USART1_UART_Init();
	MX_TIM2_Init();
	MX_I2C3_Init();
	/* USER CODE BEGIN 2 */

	HAL_TIM_Encoder_Start_IT(&htim2, TIM_CHANNEL_ALL);
	__HAL_TIM_SET_COUNTER(&htim2, ENC_KNOB_CNT_MAX);

	/* USER CODE END 2 */

	/* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
	BspCOMInit.BaudRate = 115200;
	BspCOMInit.WordLength = COM_WORDLENGTH_8B;
	BspCOMInit.StopBits = COM_STOPBITS_1;
	BspCOMInit.Parity = COM_PARITY_NONE;
	BspCOMInit.HwFlowCtl = COM_HWCONTROL_NONE;
	if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
	{
		Error_Handler();
	}

	/* USER CODE BEGIN BSP */
	ssd1306_Init();

	showPartDatasheet();
	HAL_Delay(1000);

	ssd1306_Fill(Black);
	ssd1306_SetCursor(20, 0);
	ssd1306_WriteString("ufnalski.edu.pl", Font_6x8, White);
	ssd1306_SetCursor(22, 11);
	ssd1306_WriteString("Ford Focus Mk4", Font_6x8, White);
	ssd1306_SetCursor(12, 22);
	ssd1306_WriteString("radio panel (2020)", Font_6x8, White);
	ssd1306_UpdateScreen();

	/* -- Sample board code to send message over COM1 port ---- */
	printf("\r\nWelcome to LIN bus world!\r\n");

	/* USER CODE END BSP */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	LinSendSoftTimer = HAL_GetTick();
	UartMsgSoftTimer = HAL_GetTick();
	OledSoftTimer = HAL_GetTick();

	while (1)
	{

		if ((HAL_GetTick() - UartMsgSoftTimer > UART_SEND_PERIOD)
				&& (lin_data_received_flag == 1))
		{
			lin_data_received_flag = 0;
			UartMsgSoftTimer = HAL_GetTick();
			printf("\x1B[2J\x1B[H");

			printf("\e[36m╔══════════════════════════════════╗\e[37m\r\n");
			printf(
					"\e[36m║ TX: [%02d] %02X %02X %02X                \e[36m║\e[37m\r\n",
					TX_LIN_MSG_ID, txData[2], txData[3], txData[4]);
			printf(
					"\e[36m║ RX: [%02d] %02X %02X %02X %02X %02X %02X %02X %02X \e[36m║\e[37m\r\n",
					RX_LIN_MSG_ID_A, rxDataA[3], rxDataA[4], rxDataA[5],
					rxDataA[6], rxDataA[7], rxDataA[8], rxDataA[9],
					rxDataA[10]);
			printf(
					"\e[36m║ RX: [%02d] %02X %02X %02X %02X %02X %02X %02X %02X \e[36m║\e[37m\r\n",
					RX_LIN_MSG_ID_B, rxDataB[3], rxDataB[4], rxDataB[5],
					rxDataB[6], rxDataB[7], rxDataB[8], rxDataB[9],
					rxDataB[10]);
			printf(
					"\e[36m║ RX: [%02d] %02X %02X %02X %02X %02X %02X %02X %02X \e[36m║\e[37m\r\n",
					RX_LIN_MSG_ID_C, rxDataC[3], rxDataC[4], rxDataC[5],
					rxDataC[6], rxDataC[7], rxDataC[8], rxDataC[9],
					rxDataC[10]);
			printf(
					"\e[36m║ RX: [%02d] %02X %02X %02X %02X %02X %02X %02X %02X \e[36m║\e[37m\r\n",
					RX_LIN_MSG_ID_D, rxDataD[3], rxDataD[4], rxDataD[5],
					rxDataD[6], rxDataD[7], rxDataD[8], rxDataD[9],
					rxDataD[10]);
			printf("\e[36m╚══════════════════════════════════╝\e[37m\r\n");

		}

		if (HAL_GetTick() - LinSendSoftTimer > LIN_SEND_PERIOD)
		{
			LinSendSoftTimer = HAL_GetTick();

			backlight_intensity = __HAL_TIM_GET_COUNTER(&htim2) / 4;

			// send backlight
			txData[0] = 0x55;  // sync field
			txData[1] = Pid_Calc(TX_LIN_MSG_ID);
			txData[2] = backlight_intensity; // haven't tested it precisely :)
			txData[3] = backlight_intensity;
			txData[4] = backlight_intensity;
			txData[5] = Checksum_Calc(txData[1], txData + 2, 3); // LIN 2.x includes PID
			HAL_LIN_SendBreak(&huart1);
			HAL_UART_Transmit(&huart1, txData, 6, 10);

			HAL_Delay(10);
			// ask for diagnostics
			txData[0] = 0x55;

			switch (rx_msg_index)
			{
			case 0:
				txData[1] = Pid_Calc(RX_LIN_MSG_ID_A);
				memset(rxDataA, 0x00, RX_BUFFER_SIZE_A);
				HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxDataA,
				RX_BUFFER_SIZE_A);
				break;
			case 1:
				txData[1] = Pid_Calc(RX_LIN_MSG_ID_B);
				memcpy(rxDataBbis, rxDataB, RX_BUFFER_SIZE_B);
				memset(rxDataB, 0x00, RX_BUFFER_SIZE_B);
				HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxDataB,
				RX_BUFFER_SIZE_B);
				break;
			case 2:
				txData[1] = Pid_Calc(RX_LIN_MSG_ID_C);
				memset(rxDataC, 0x00, RX_BUFFER_SIZE_C);
				HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxDataC,
				RX_BUFFER_SIZE_C);
				break;
			case 3:
				txData[1] = Pid_Calc(RX_LIN_MSG_ID_D);
				memset(rxDataD, 0x00, RX_BUFFER_SIZE_D);
				HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rxDataD,
				RX_BUFFER_SIZE_D);
				break;
			default:
				__NOP();
			}

			__HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
			rx_msg_index++;
			rx_msg_index %= NUMBER_OF_RX_MSGS;

			HAL_LIN_SendBreak(&huart1);
			HAL_UART_Transmit_DMA(&huart1, txData, 2);
		}

		if (HAL_GetTick() - OledSoftTimer > OLED_UPDATE_PERIOD)
		{
			lin_data_received_flag = 0;
			OledSoftTimer = HAL_GetTick();
			sprintf(lcd_line, "TX: 0b"BYTE_TO_BINARY_PATTERN,
					BYTE_TO_BINARY(backlight_intensity));
			ssd1306_SetCursor(2, 33);
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			ssd1306_SetCursor(2, 44);
			ssd1306_WriteString("RX:", Font_6x8, White);
			sprintf(lcd_line, " %02X%02X %02X%02X %02X%02X %02X%02X",
					rxDataBbis[3], rxDataBbis[4], rxDataBbis[5], rxDataBbis[6],
					rxDataBbis[7], rxDataBbis[8], rxDataBbis[9],
					rxDataBbis[10]);
			ssd1306_SetCursor(2, 55);
			ssd1306_WriteString(lcd_line, Font_6x8, White);
			ssd1306_UpdateScreen();
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
	RCC_OscInitTypeDef RCC_OscInitStruct =
	{ 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
	{ 0 };

	/** Configure the main internal regulator output voltage
	 */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		lin_data_received_flag = 1;
		if (rx_msg_index % NUMBER_OF_RX_MSGS == 0)
		{
			HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

	if (htim->Instance == TIM2)
	{
		if ((__HAL_TIM_GET_COUNTER(&htim2) < ENC_KNOB_CNT_MIN)
				|| (__HAL_TIM_GET_COUNTER(&htim2) > ENC_KNOB_CNT_MAX * 2))
		{
			__HAL_TIM_SET_COUNTER(&htim2, ENC_KNOB_CNT_MIN);
		}
		else if (__HAL_TIM_GET_COUNTER(&htim2) > ENC_KNOB_CNT_MAX)
		{
			__HAL_TIM_SET_COUNTER(&htim2, ENC_KNOB_CNT_MAX);
		}
	}

}

uint8_t Pid_Calc(uint8_t ID)
{
	if (ID > 0x3F)
		Error_Handler();
	uint8_t IDBuf[6];
	for (int i = 0; i < 6; i++)
	{
		IDBuf[i] = (ID >> i) & 0x01;
	}

	uint8_t P0 = (IDBuf[0] ^ IDBuf[1] ^ IDBuf[2] ^ IDBuf[4]) & 0x01;
	uint8_t P1 = ~((IDBuf[1] ^ IDBuf[3] ^ IDBuf[4] ^ IDBuf[5]) & 0x01);

	ID = ID | (P0 << 6) | (P1 << 7);
	return ID;
}

uint8_t Checksum_Calc(uint8_t PID, uint8_t *data, uint8_t size)
{
	uint8_t buffer[size + 2];
	uint16_t sum = 0;
	buffer[0] = PID;
	for (int i = 0; i < size; i++)
	{
		buffer[i + 1] = data[i];
	}

	for (int i = 0; i < size + 1; i++)
	{
		sum = sum + buffer[i];
		if (sum > 0xff)
			sum = sum - 0xff;
	}

	sum = 0xff - sum;
	return sum;
}

static void showPartDatasheet(void)
{
	//const char *text = "Enjoy your life. There is no second take on it.";
	const char *text = "https://allegro.pl/listing?string=JX7T-18K811-CC";
	enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level

	// Make and print the QR Code symbol
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
	uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
	bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
	qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
	if (ok)
		printQr(qrcode);
	displayQr(qrcode);

}

static void printQr(const uint8_t qrcode[])
{
	int size = qrcodegen_getSize(qrcode);
	int border = 4;
	for (int y = -border; y < size + border; y++)
	{
		for (int x = -border; x < size + border; x++)
		{
			printf("%s", qrcodegen_getModule(qrcode, x, y) ? "##" : "  ");
		}
		printf("\r\n");
	}
	printf("\r\n");
}

static void displayQr(const uint8_t qrcode[])
{
	int size = qrcodegen_getSize(qrcode);
	int border = 0;
	ssd1306_Fill(Black);
	for (int y = -border; y < size + border; y++)
	{
		for (int x = -border; x < size + border; x++)
		{
			if (qrcodegen_getModule(qrcode, x, y) == 1)
			{
				ssd1306_DrawPixel(2 * x + 40, 2 * y, White);
				ssd1306_DrawPixel(2 * x + 1 + 40, 2 * y + 1, White);
				ssd1306_DrawPixel(2 * x + 40, 2 * y + 1, White);
				ssd1306_DrawPixel(2 * x + 1 + 40, 2 * y, White);

				//ssd1306_FillRectangle(x, y, x+1, y+1,White);
			}
			//	printf("%s",qrcodegen_getModule(qrcode, x, y) ? "##" : "  ");
		}
		//	printf("\r\n");
	}
//	printf("\r\n");
	ssd1306_UpdateScreen();
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/**
 * @}
 */

/**
 * @}
 */

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
