/**
 ******************************************************************************
 * File Name          : main.c
 * Description        : Main program body
 ******************************************************************************
 ** This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * COPYRIGHT(c) 2018 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "printf2.h"
#include "stm32l4xx_hal.h"
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "crc32.h"

/* USER CODE BEGIN Includes */
#include "config.h"
#include "fw_update.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_CRC_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
typedef  void (*pFunction)(void);
static bool inDFU = false;

void jump_to_application()
{
  HAL_UART_MspDeInit(&huart1);
  HAL_CRC_MspDeInit(&hcrc);

  /// Initialize user application's Stack Pointer
  __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);

  /// Reset interrupt vector to application
  SCB->VTOR = 0x08000000 | ISR_VECTOR_OFFSET;

  // Start the application
  pFunction start = (pFunction)(*(__IO uint32_t*) (APPLICATION_ADDRESS + 4));
  start();
}


uint32_t imageCRC32(Metadata *fw)
{
  return crc32((void*)APPLICATION_ADDRESS, fw->size);
}

bool rescue_requested()
{
  uint32_t now = HAL_GetTick();
  int toggleCount = 0;
  int oldState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);
  while ( HAL_GetTick() - now < 2500 )
    {
      int newState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12);
      if ( oldState != newState )
        {
          ++toggleCount;
          oldState = newState;
        }

      if ( toggleCount > 1 )
        return true;

      __WFI();
    }

  return false;
}

void main_tick()
{
  // TODO: Anything?
}

/* USER CODE END 0 */

int main(void)
{
  firmwareUpdate.state = WAITING;
  GPIO_InitTypeDef gpio;

  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_CRC_Init();
  bool hasValidFirmware = false;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) != RESET)
    {
      // This is a power-on (or brown-out) reset
      // TX switch can be used for rescue mode immediately after POR
      gpio.Pin = GPIO_PIN_12;
      gpio.Mode = GPIO_MODE_INPUT;
      gpio.Pull = GPIO_PULLUP;
      HAL_GPIO_Init(GPIOA, &gpio);

      __HAL_RCC_CLEAR_RESET_FLAGS();

      if ( rescue_requested() )
        goto enter_dfu;
    }

  // Since there is no "rescue" mode request, now we check for explicit DFU request via reserved RAM
  if ( *(uint32_t*)BOOTMODE_ADDRESS == DFU_FLAG_MAGIC )
    {
      *(uint32_t*)BOOTMODE_ADDRESS = 0;
      goto enter_dfu;
    }

  Metadata *fw = (Metadata*)METADATA_ADDRESS;
  if ( fw->magic == 0xabadbabe )
    {
      uint32_t crc = imageCRC32(fw);
      hasValidFirmware = (crc == fw->crc32);
    }

  // If there's firmware installed, jump_to_application to it -- do nothing else
  if ( hasValidFirmware )
    jump_to_application();

  /*
   * If we get here, we're waiting for a firmware upload
   */

enter_dfu:

  // Turn on all LEDs to indicate this
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_LOW;
  gpio.Alternate = 0;
  gpio.Pin = GPIO_PIN_1|GPIO_PIN_11;

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1|GPIO_PIN_11, GPIO_PIN_SET);
  HAL_GPIO_Init(GPIOA, &gpio);

  gpio.Pin = GPIO_PIN_5;
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
  HAL_GPIO_Init(GPIOB, &gpio);

  inDFU = true;
  MX_USART1_UART_Init();
  dfu_init();

  while (1)
    {
      __WFI();
    }
}

/** System Clock Configuration
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /**Initializes the CPU, AHB and APB bus clocks
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;  // 80 MHz
#if defined(STM32L432xx) || defined(STM32L431xx)
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
#endif
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

  /**Initializes the CPU, AHB and APB bus clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
      |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
      Error_Handler();
    }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
  //PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }

  /**Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
      Error_Handler();
    }

  /**Configure the Systick interrupt time
   */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  /**Configure the Systick
   */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* CRC init function */
static void MX_CRC_Init(void)
{
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.CRCLength = CRC_POLYLENGTH_32B;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 38400;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

}

/** Configure pins as 
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
static void MX_GPIO_Init(void)
{

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
#if ENABLE_VAR_ARGS
  printf2("[BOOT ERROR] %s:%d\r\n", file, line);
#endif

  while(1) 
    {
    }
  /* USER CODE END Error_Handler_Debug */ 
}

void HAL_SYSTICK_Callback(void)
{
  static uint32_t count = 0;
  if ( count++ % 1000 == 0 )
    {
      if ( inDFU )
        dfu_tick();
      else
        main_tick();
    }
}

#ifdef USE_FULL_ASSERT

/**
 * @brief Reports the name of the source file and the source line number
 * where the assert_param error has occurred.
 * @param file: pointer to the source file name
 * @param line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
