/*
  Copyright (c) 2016-2020 Peter Antypas

  This file is part of the MAIANA™ transponder firmware.

  The firmware is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>
*/



#include "bsp.hpp"
#include <stm32l4xx_hal.h>
#include "printf_serial.h"
#include <string.h>


#if BOARD_REV==93

I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
IWDG_HandleTypeDef hiwdg;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart1;
TIM_HandleTypeDef htim2;

void SystemClock_Config();

char_input_cb gnssInputCallback = nullptr;
char_input_cb terminalInputCallback = nullptr;
irq_callback ppsCallback = nullptr;
irq_callback sotdmaCallback = nullptr;
irq_callback trxClockCallback = nullptr;
irq_callback rxClockCallback = nullptr;
irq_callback tickCallback = nullptr;


typedef struct
{
  GPIO_TypeDef *port;
  GPIO_InitTypeDef gpio;
  GPIO_PinState init;
} GPIO;

static const GPIO __gpios[] = {
    {GNSS_EN_PORT, {GNSS_EN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {TRX_IC_CLK_PORT, {TRX_IC_CLK_PIN, GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {TX_DISABLE_PORT, {TX_DISABLE_PIN, GPIO_MODE_INPUT, GPIO_PULLUP, GPIO_SPEED_LOW, 0}, GPIO_PIN_SET},
    {CS2_PORT, {CS2_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, 0}, GPIO_PIN_SET},
    {RX_EVT_PORT, {RX_EVT_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {GNSS_1PPS_PORT, {GNSS_1PPS_PIN, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {GNSS_NMEA_RX_PORT, {GNSS_NMEA_RX_PIN, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_LOW, GPIO_AF7_USART2}, GPIO_PIN_RESET},
    {CS1_PORT, {CS1_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, 0}, GPIO_PIN_SET},
    {SCK_PORT, {SCK_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF5_SPI1}, GPIO_PIN_SET},
    {MISO_PORT, {MISO_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF5_SPI1}, GPIO_PIN_SET},
    {MOSI_PORT, {MOSI_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_HIGH, GPIO_AF5_SPI1}, GPIO_PIN_SET},
    {SDN1_PORT, {SDN1_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_SET},
    {TRX_IC_DATA_PORT, {TRX_IC_DATA_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {TX_EVT_PORT, {TX_EVT_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {UART_TX_PORT, {UART_TX_PIN, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_LOW, GPIO_AF7_USART1}, GPIO_PIN_RESET},
    {UART_RX_PORT, {UART_RX_PIN, GPIO_MODE_AF_PP, GPIO_PULLUP, GPIO_SPEED_LOW, GPIO_AF7_USART1}, GPIO_PIN_RESET},
    {GNSS_STATE_PORT, {GNSS_STATE_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {SDN2_PORT, {SDN2_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_SET},
    {RX_IC_CLK_PORT, {RX_IC_CLK_PIN, GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {RX_IC_DATA_PORT, {RX_IC_DATA_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {PA_BIAS_PORT, {PA_BIAS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {I2C_SCL_PORT, {I2C_SCL_PIN, GPIO_MODE_AF_OD, GPIO_PULLUP, GPIO_SPEED_HIGH, GPIO_AF4_I2C1}, GPIO_PIN_SET},
    {I2C_SDA_PORT, {I2C_SDA_PIN, GPIO_MODE_AF_OD, GPIO_PULLUP, GPIO_SPEED_HIGH, GPIO_AF4_I2C1}, GPIO_PIN_SET},
};

extern "C"
{
  void Error_Handler(uint8_t i)
  {
    asm("BKPT 0");
    printf_serial_now("[ERROR %d]\r\n", i);
    //printf_serial_now("[ERROR] ***** System error handler resetting *****\r\n");
    //NVIC_SystemReset();
  }
}


void gpio_pin_init();

void bsp_hw_init()
{
  HAL_Init();
  SystemClock_Config();

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_SPI1_CLK_ENABLE();
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_I2C1_CLK_ENABLE();

  gpio_pin_init();



  // USART1 (main UART)
  huart1.Instance                     = USART1;
  huart1.Init.BaudRate                = 38400;
  huart1.Init.WordLength              = UART_WORDLENGTH_8B;
  huart1.Init.StopBits                = UART_STOPBITS_1;
  huart1.Init.Parity                  = UART_PARITY_NONE;
  huart1.Init.Mode                    = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl               = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling            = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling          = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit  = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);

  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);


  // SPI

  hspi1.Instance                = SPI1;
  hspi1.Init.Mode               = SPI_MODE_MASTER;
  hspi1.Init.Direction          = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize           = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity        = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase           = SPI_PHASE_1EDGE;
  hspi1.Init.NSS                = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit           = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode             = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial      = 7;
  hspi1.Init.CRCLength          = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode           = SPI_NSS_PULSE_DISABLE;

  if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
      Error_Handler(0);
    }

  __HAL_SPI_ENABLE(&hspi1);


  // USART2 (GNSS, RX only)
  huart2.Instance                     = USART2;
  huart2.Init.BaudRate                = 9600;
  huart2.Init.WordLength              = UART_WORDLENGTH_8B;
  huart2.Init.StopBits                = UART_STOPBITS_1;
  huart2.Init.Parity                  = UART_PARITY_NONE;
  huart2.Init.Mode                    = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl               = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling            = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling          = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit  = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart2);

  HAL_NVIC_SetPriority(USART2_IRQn, 7, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);

  // TIM2 for SOTDMA (37.5Hz)
  uint32_t period = (SystemCoreClock / 37.5) - 1;

  __HAL_RCC_TIM2_CLK_ENABLE();
  htim2.Instance               = TIM2;
  htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.Prescaler         = 0;
  htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
  htim2.Init.Period            = period;
  htim2.Init.RepetitionCounter = 0;

  HAL_TIM_Base_Init(&htim2);

  // I2C
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00702991;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
      Error_Handler(0);
    }
  /** Configure Analogue filter
   */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
    {
      Error_Handler(0);
    }
  /** Configure Digital filter
   */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
    {
      Error_Handler(0);
    }

  // 1PPS signal
  HAL_NVIC_SetPriority(EXTI2_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  // SOTDMA
  HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);

  // RF IC clock interrupts
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  // This is our HAL tick timer now
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
}



void SystemClock_Config()
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
  //RCC_OscInitStruct.PLL.PLLN = 8; // 64 MHz
  //RCC_OscInitStruct.PLL.PLLN = 6; // 48 MHz
#ifdef STM32L432xx
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
#endif
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      Error_Handler(0);
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
      Error_Handler(0);
    }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler(0);
    }

  /**Configure the main internal regulator output voltage
   */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
      Error_Handler(0);
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

void gpio_pin_init()
{
  for ( unsigned i = 0; i < sizeof __gpios / sizeof(GPIO); ++i )
    {
      const GPIO* io = &__gpios[i];
      HAL_GPIO_Init(io->port, (GPIO_InitTypeDef*)&io->gpio);
      if ( io->gpio.Mode == GPIO_MODE_OUTPUT_PP || io->gpio.Mode == GPIO_MODE_OUTPUT_OD )
        {
          HAL_GPIO_WritePin(io->port, io->gpio.Pin, io->init);
        }

    }
}

void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /**
   * Some of these interrupts will be managed and configured in FreeRTOS
   */

  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
  /* BusFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
  /* UsageFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
  /* SVCall_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SVCall_IRQn, 10, 0);
  /* DebugMonitor_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 10, 0);
  /* SysTick_IRQn interrupt configuration */

#ifndef RTOS
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
#endif

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

void bsp_set_rx_mode()
{
  HAL_GPIO_WritePin(PA_BIAS_PORT, PA_BIAS_PIN, GPIO_PIN_RESET);       // Kill the RF MOSFET bias voltage

  GPIO_InitTypeDef gpio;
  gpio.Pin = TRX_IC_DATA_PIN;
  gpio.Mode = GPIO_MODE_INPUT;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TRX_IC_DATA_PORT, &gpio);
}

void bsp_set_tx_mode()
{
  GPIO_InitTypeDef gpio;
  gpio.Pin = TRX_IC_DATA_PIN;
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(TRX_IC_DATA_PORT, &gpio);

  HAL_GPIO_WritePin(PA_BIAS_PORT, PA_BIAS_PIN, GPIO_PIN_SET);       // RF MOSFET bias voltage
}

void bsp_rx_led_on()
{
  HAL_GPIO_WritePin(RX_EVT_PORT, RX_EVT_PIN, GPIO_PIN_SET);
}

void bsp_rx_led_off()
{
  HAL_GPIO_WritePin(RX_EVT_PORT, RX_EVT_PIN, GPIO_PIN_RESET);
}

void bsp_tx_led_on()
{
  HAL_GPIO_WritePin(TX_EVT_PORT, TX_EVT_PIN, GPIO_PIN_SET);
}

void bsp_tx_led_off()
{
  HAL_GPIO_WritePin(TX_EVT_PORT, TX_EVT_PIN, GPIO_PIN_RESET);
}

void bsp_gps_led_on()
{
  HAL_GPIO_WritePin(GNSS_STATE_PORT, GNSS_STATE_PIN, GPIO_PIN_SET);
}

void bsp_gps_led_off()
{
  HAL_GPIO_WritePin(GNSS_STATE_PORT, GNSS_STATE_PIN, GPIO_PIN_RESET);
}

void bsp_gnss_on()
{
  HAL_GPIO_WritePin(GNSS_EN_PORT, GNSS_EN_PIN, GPIO_PIN_SET);
}

void bsp_gnss_off()
{
  HAL_GPIO_WritePin(GNSS_EN_PORT, GNSS_EN_PIN, GPIO_PIN_RESET);
}


void USART_putc(USART_TypeDef* USARTx, char c)
{
  while ( !(USARTx->ISR & USART_ISR_TXE) )
    ;

  USARTx->TDR = c;
}

void bsp_write_char(char c)
{
  USART_putc(USART1, c);
}

void bsp_write_string(const char *s)
{
  for ( int i = 0; s[i] != 0; ++i )
    USART_putc(USART1, s[i]);
}

void bsp_start_wdt()
{
  IWDG_InitTypeDef iwdg;
  iwdg.Prescaler = IWDG_PRESCALER_64;
  iwdg.Reload = 0x0fff;
  iwdg.Window = 0x0fff;

  hiwdg.Instance = IWDG;
  hiwdg.Init = iwdg;

  HAL_IWDG_Init(&hiwdg);
}

void bsp_refresh_wdt()
{
  HAL_IWDG_Refresh(&hiwdg);
}

void bsp_set_gnss_input_callback(char_input_cb cb)
{
  gnssInputCallback = cb;
}

void bsp_set_terminal_input_callback(char_input_cb cb)
{
  terminalInputCallback = cb;
}

void bsp_start_sotdma_timer()
{
  HAL_TIM_Base_Start_IT(&htim2);
}

void bsp_stop_sotdma_timer()
{
  HAL_TIM_Base_Stop_IT(&htim2);
}

void bsp_set_gnss_1pps_callback(irq_callback cb)
{
  ppsCallback = cb;
}

void bsp_set_trx_clk_callback(irq_callback cb)
{
  trxClockCallback = cb;
}

void bsp_set_rx_clk_callback(irq_callback cb)
{
  rxClockCallback = cb;
}

void bsp_set_gnss_sotdma_timer_callback(irq_callback cb)
{
  sotdmaCallback = cb;
}

void bsp_set_tick_callback(irq_callback cb)
{
  tickCallback = cb;
}

uint32_t bsp_get_sotdma_timer_value()
{
  return TIM2->CNT;
}

void bsp_set_sotdma_timer_value(uint32_t v)
{
  TIM2->CNT = v;
}

uint32_t bsp_get_system_clock()
{
  return SystemCoreClock;
}

uint8_t bsp_tx_spi_byte(uint8_t data)
{
  uint8_t result = 0;
  HAL_SPI_TransmitReceive(&hspi1, &data, &result, 1, 2);
  return result;
}

void bsp_reboot()
{
  NVIC_SystemReset();
}

bool bsp_is_tx_disabled()
{
  return HAL_GPIO_ReadPin(TX_DISABLE_PORT, TX_DISABLE_PIN) == GPIO_PIN_RESET;
}

void bsp_enter_dfu()
{
  // Cut off the GPS signals immediately to prevent its UART from transmitting and hijacking the bootloader upon reset
  bsp_gnss_off();

  HAL_Delay(1000);

  // This flag simply tells main() to jump to the ROM bootloader immediately upon reset, before initializing anything
  *(uint32_t*)BOOTMODE_ADDRESS = DFU_FLAG_MAGIC;

  bsp_reboot();
}

void bsp_signal_rx_event()
{
  HAL_GPIO_WritePin(RX_EVT_PORT, RX_EVT_PIN, GPIO_PIN_SET);
}

void bsp_signal_tx_event()
{
  HAL_GPIO_WritePin(TX_EVT_PORT, TX_EVT_PIN, GPIO_PIN_SET);
}

void bsp_signal_gps_status(bool tracking)
{
  HAL_GPIO_WritePin(GNSS_STATE_PORT, GNSS_STATE_PIN, tracking ? GPIO_PIN_SET: GPIO_PIN_RESET);
}

extern "C"
{

  void USART1_IRQHandler(void)
  {
    if ( __HAL_UART_GET_IT(&huart1, UART_IT_RXNE) )
      {
        __HAL_UART_CLEAR_IT(&huart1, UART_IT_RXNE);
        char c = USART1->RDR;
        if ( terminalInputCallback )
          terminalInputCallback(c);
      }
  }

  void EXTI2_IRQHandler(void)
  {
    if ( __HAL_GPIO_EXTI_GET_IT(GPIO_PIN_2) != RESET )
      {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
        if ( ppsCallback )
          ppsCallback();
      }
  }

  void USART2_IRQHandler()
  {
    if ( __HAL_UART_GET_IT(&huart2, UART_IT_RXNE) )
      {
        __HAL_UART_CLEAR_IT(&huart2, UART_IT_RXNE);
        char c = (char)USART2->RDR;
        if ( gnssInputCallback )
          gnssInputCallback(c);
      }
  }


  void TIM2_IRQHandler(void)
  {
    if(__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET)
      {
        if(__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) !=RESET)
          {
            __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
            if ( sotdmaCallback )
              sotdmaCallback();
          }
      }
  }

  void EXTI3_IRQHandler(void)
  {
    if ( __HAL_GPIO_EXTI_GET_IT(GPIO_PIN_3) != RESET )
      {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
        if ( rxClockCallback )
          rxClockCallback();
      }
  }

  void EXTI15_10_IRQHandler(void)
  {
    if ( __HAL_GPIO_EXTI_GET_IT(GPIO_PIN_15) != RESET )
      {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);
        if ( trxClockCallback )
          trxClockCallback();
      }
  }

  void HAL_SYSTICK_Callback()
  {
    if ( tickCallback )
      tickCallback();
  }

}

#endif
