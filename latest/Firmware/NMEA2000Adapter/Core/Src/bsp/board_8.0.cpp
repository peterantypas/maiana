/*
 * board_1.cpp
 *
 *  Created on: Jun 28, 2021
 *      Author: peter
 */

#include <bsp/bsp.hpp>
#include <stm32g0xx_hal.h>
#include <stdio.h>


#define CS_PORT                   GPIOA
#define CS_PIN                    GPIO_PIN_2

#define NMEA_IN_PORT              GPIOA
#define NMEA_IN_PIN               GPIO_PIN_3

#define SCK_PORT                  GPIOA
#define SCK_PIN                   GPIO_PIN_5

#define MISO_PORT                 GPIOA
#define MISO_PIN                  GPIO_PIN_6

#define MOSI_PORT                 GPIOA
#define MOSI_PIN                  GPIO_PIN_7

#define UART_TX_PORT              GPIOA
#define UART_TX_PIN               GPIO_PIN_9

#define UART_RX_PORT              GPIOA
#define UART_RX_PIN               GPIO_PIN_10

#define NMEA_EN_PORT              GPIOB
#define NMEA_EN_PIN               GPIO_PIN_0

#define GREENPAK_RESET_PORT       GPIOB
#define GREENPAK_RESET_PIN        GPIO_PIN_3


#define CONFIG_ADDRESS            0x0801F800
#define CONFIG_MAGIC              0x313DEEF6


typedef struct
{
  uint32_t magic;
  uint32_t address;
} CANConfig;


typedef union
{
  CANConfig can;
  uint64_t dw[128];
} ConfigPage;

static ConfigPage __page;

typedef struct
{
  GPIO_TypeDef *port;
  GPIO_InitTypeDef gpio;
  GPIO_PinState init;
} GPIO;

static const GPIO __gpios[] = {
    {CS_PORT, {CS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0}, GPIO_PIN_SET},
    {SCK_PORT, {SCK_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF0_SPI1}, GPIO_PIN_SET},
    {MISO_PORT, {MISO_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF0_SPI1}, GPIO_PIN_SET},
    {MOSI_PORT, {MOSI_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, GPIO_AF0_SPI1}, GPIO_PIN_SET},
    //{UART_TX_PORT, {UART_TX_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    //{UART_RX_PORT, {UART_RX_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_LOW, 0}, GPIO_PIN_RESET},
    {NMEA_IN_PORT, {NMEA_IN_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM, GPIO_AF1_USART2}, GPIO_PIN_RESET},
    //{GREENPAK_RESET_PORT, {GREENPAK_RESET_PIN, GPIO_MODE_OUTPUT_OD, GPIO_SPEED_MEDIUM, 0}, GPIO_PIN_SET},
    {NMEA_EN_PORT, {NMEA_EN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0}, GPIO_PIN_SET},

};


SPI_HandleTypeDef hspi1;
//UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
//TIM_HandleTypeDef htim6;
IWDG_HandleTypeDef hiwdg;

void gpio_pin_init();
void SystemClock_Config(void);
void Error_Handler();

uart_irq_callback usart_irq = nullptr;
can_irq_callback can_irq = nullptr;

void bsp_set_uart_irq_cb(uart_irq_callback cb)
{
  usart_irq = cb;
}

void bsp_start_wdt()
{
  IWDG_InitTypeDef iwdg;
  iwdg.Prescaler = IWDG_PRESCALER_8;
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

#if 0
void bsp_set_can_irq_cb(can_irq_callback cb)
{
  can_irq = cb;
}
#endif

uint32_t bsp_dwt_init(void)
{

#if 0
  /* Disable TRC */
  CoreDebug->DEMCR &= ~CoreDebug_DEMCR_TRCENA_Msk; // ~0x01000000;
  /* Enable TRC */
  CoreDebug->DEMCR |=  CoreDebug_DEMCR_TRCENA_Msk; // 0x01000000;

  /* Disable clock cycle counter */
  DWT->CTRL &= ~DWT_CTRL_CYCCNTENA_Msk; //~0x00000001;
  /* Enable  clock cycle counter */
  DWT->CTRL |=  DWT_CTRL_CYCCNTENA_Msk; //0x00000001;

  /* Reset the clock cycle counter value */
  DWT->CYCCNT = 0;

  /* 3 NO OPERATION instructions */
  __ASM volatile ("NOP");
  __ASM volatile ("NOP");
  __ASM volatile ("NOP");

  /* Check if clock cycle counter has started */
  if(DWT->CYCCNT)
    {
      return 0; /*clock cycle counter started*/
    }
  else
    {
      return 1; /*clock cycle counter not started*/
    }
#endif

  return 0;
}


void bsp_init()
{
  // To redirect printf() to serial, disable buffering first
  //setvbuf(stdout, NULL, _IONBF, 0);
  //setvbuf(stderr, NULL, _IONBF, 0);

  HAL_Init();

  SystemClock_Config();

  __HAL_RCC_SPI1_CLK_ENABLE();
  //__HAL_RCC_USART1_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  gpio_pin_init();


  // SPI

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
      Error_Handler();
    }
  __HAL_SPI_ENABLE(&hspi1);

  //HAL_NVIC_EnableIRQ(EXTI0_IRQn);

#if 0

  // UART1
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
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
      Error_Handler();
    }

  HAL_NVIC_EnableIRQ(USART1_IRQn);
  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
#endif


  // UART2
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
    {
      Error_Handler();
    }

  HAL_NVIC_EnableIRQ(USART2_IRQn);
  HAL_NVIC_SetPriority(USART2_IRQn, 1, 0);
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);


  // We'll be using the Cortex M4 cycle counter for microsecond delays
  bsp_dwt_init();

#if 0
  HAL_GPIO_WritePin(GREENPAK_RESET_PORT, GREENPAK_RESET_PIN, GPIO_PIN_RESET);
  bsp_delay_us(300);
  HAL_GPIO_WritePin(GREENPAK_RESET_PORT, GREENPAK_RESET_PIN, GPIO_PIN_SET);
#endif
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

bool bsp_is_isr()
{
  return __get_IPSR();
}

void bsp_wfi()
{
  __WFI();
}

void bsp_mcp_2515_select()
{
  HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
}

void bsp_mcp_2515_unselect()
{
  HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
}

uint8_t bsp_spi_transfer(uint8_t byte)
{
  uint8_t result = 0;
  HAL_SPI_TransmitReceive(&hspi1, &byte, &result, 1, 10);
  return result;
}

void bsp_delay_us(uint32_t us)
{
#if 0
  static uint32_t ticks_per_us = (HAL_RCC_GetHCLKFreq() / 1000000);

  DWT->CYCCNT = 0;
  while (DWT->CYCCNT < us*ticks_per_us)
    ;
#endif
}

uint8_t bsp_last_can_address()
{
  ConfigPage *cfg = (ConfigPage*)CONFIG_ADDRESS;
  if ( cfg->can.magic == CONFIG_MAGIC )
    return cfg->can.address & 0xff;

  return 15;
}

bool bsp_erase_config_page()
{
#warning "Fix me"
#if 0
  uint32_t page = (CONFIG_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE;

  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();

  FLASH_EraseInitTypeDef erase;
  erase.TypeErase = FLASH_TYPEERASE_PAGES;
  erase.Banks     = FLASH_BANK_1;
  erase.Page      = page;
  erase.NbPages   = 1;

  uint32_t errPage;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &errPage);
  if ( status != HAL_OK )
    {
      HAL_FLASH_Lock();
      return false;
    }

  HAL_FLASH_Lock();
#endif

  return true;
}

void bsp_save_can_address(uint8_t address)
{
#warning "Fix me"
#if 0
  __page.can.magic = CONFIG_MAGIC;
  __page.can.address = address;

  uint32_t pageAddress = CONFIG_ADDRESS;
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  HAL_FLASH_Unlock();
  HAL_StatusTypeDef status = HAL_OK;
  for ( uint32_t dw = 0; dw < sizeof __page/8; ++dw )
    {
      status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, pageAddress + dw*8, __page.dw[dw]);
      if ( status != HAL_OK )
        break;
    }
  HAL_FLASH_Lock();
#endif
}

extern "C" {

#if 0
  int _write(int fd, char* ptr, int len)
  {
    HAL_StatusTypeDef hstatus;

    hstatus = HAL_UART_Transmit(&huart1, (uint8_t *) ptr, len, HAL_MAX_DELAY);
    if (hstatus == HAL_OK)
      return len;
    else
      return -1;
  }
#endif

  // delay() and millis() for compatibility with Arduino

  void delay(uint32_t ms)
  {
    HAL_Delay(ms);
  }

  uint32_t millis()
  {
    return HAL_GetTick();
  }

  void USART2_IRQHandler(void)
  {
    if ( __HAL_UART_GET_IT(&huart2, UART_IT_RXNE) )
      {
        __HAL_UART_CLEAR_IT(&huart2, UART_IT_RXNE);
        char c = USART2->RDR;
        if ( usart_irq )
          usart_irq(c);
      }
  }

#if 0
  void USART1_IRQHandler(void)
  {
    if ( __HAL_UART_GET_IT(&huart1, UART_IT_RXNE) )
      {
        __HAL_UART_CLEAR_IT(&huart1, UART_IT_RXNE);
      }
  }
#endif

  void SysTick_Handler(void)
  {
    HAL_IncTick();
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}


void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/

  /** Disable the internal Pull-Up in Dead Battery pins of UCPD peripheral
  */
  HAL_SYSCFG_StrobeDBattpinsConfig(SYSCFG_CFGR1_UCPD1_STROBE);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}



/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  //GPIO io = {UART_TX_PORT, {UART_TX_PIN, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_LOW, GPIO_AF7_USART1}, GPIO_PIN_RESET};
  //HAL_GPIO_Init(io.port, (GPIO_InitTypeDef*)&io.gpio);

  __disable_irq();
  asm("bkpt 0");

  //printf("FATAL ERROR\n");

  while (1)
    {
    }
  /* USER CODE END Error_Handler_Debug */
}

