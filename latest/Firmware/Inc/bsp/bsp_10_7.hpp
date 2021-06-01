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

#ifndef INC_BSP_10_7_HPP_
#define INC_BSP_10_7_HPP_



// GPIO Pin definitions
#define GNSS_EN_PORT              GPIOC
#define GNSS_EN_PIN               GPIO_PIN_14

#define TRX_IC_CLK_PORT           GPIOC
#define TRX_IC_CLK_PIN            GPIO_PIN_15

#define CS2_PORT                  GPIOA
#define CS2_PIN                   GPIO_PIN_0

#define RX_EVT_PORT               GPIOA
#define RX_EVT_PIN                GPIO_PIN_1

#define GNSS_1PPS_PORT            GPIOA
#define GNSS_1PPS_PIN             GPIO_PIN_2

#define GNSS_NMEA_RX_PORT         GPIOA
#define GNSS_NMEA_RX_PIN          GPIO_PIN_3

#define CS1_PORT                  GPIOA
#define CS1_PIN                   GPIO_PIN_4

#define SCK_PORT                  GPIOA
#define SCK_PIN                   GPIO_PIN_5

#define MISO_PORT                 GPIOA
#define MISO_PIN                  GPIO_PIN_6

#define MOSI_PORT                 GPIOA
#define MOSI_PIN                  GPIO_PIN_7

#define SDN1_PORT                 GPIOB
#define SDN1_PIN                  GPIO_PIN_0

#define TRX_IC_DATA_PORT          GPIOB
#define TRX_IC_DATA_PIN           GPIO_PIN_1

#define PA_BIAS_PORT              GPIOA
#define PA_BIAS_PIN               GPIO_PIN_8

#define UART_TX_PORT              GPIOA
#define UART_TX_PIN               GPIO_PIN_9

#define UART_RX_PORT              GPIOA
#define UART_RX_PIN               GPIO_PIN_10

#define GNSS_STATE_PORT           GPIOA
#define GNSS_STATE_PIN            GPIO_PIN_11

#define TX_DISABLE_PORT           GPIOA
#define TX_DISABLE_PIN            GPIO_PIN_12

#define SDN2_PORT                 GPIOA
#define SDN2_PIN                  GPIO_PIN_15

#define RX_IC_CLK_PORT            GPIOB
#define RX_IC_CLK_PIN             GPIO_PIN_3

#define RX_IC_DATA_PORT           GPIOB
#define RX_IC_DATA_PIN            GPIO_PIN_4

#define TX_EVT_PORT               GPIOB
#define TX_EVT_PIN                GPIO_PIN_5

#define LNA_PWR_PORT              GPIOB
#define LNA_PWR_PIN               GPIO_PIN_6

#define RFSW_CTRL_PORT            GPIOB
#define RFSW_CTRL_PIN             GPIO_PIN_7


#endif /* INC_BSP_5_0_HPP_ */
