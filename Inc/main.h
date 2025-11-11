/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RF_CS_Pin GPIO_PIN_0
#define RF_CS_GPIO_Port GPIOA
#define RF_MISO_Pin GPIO_PIN_1
#define RF_MISO_GPIO_Port GPIOA
#define RF_SCLK_Pin GPIO_PIN_2
#define RF_SCLK_GPIO_Port GPIOA
#define RF_MOSI_Pin GPIO_PIN_3
#define RF_MOSI_GPIO_Port GPIOA
#define ADC_VBAT_Pin GPIO_PIN_5
#define ADC_VBAT_GPIO_Port GPIOC
#define DP1_Pin GPIO_PIN_2
#define DP1_GPIO_Port GPIOB
#define LCD_RESET_Pin GPIO_PIN_10
#define LCD_RESET_GPIO_Port GPIOB
#define SC2_Pin GPIO_PIN_11
#define SC2_GPIO_Port GPIOB
#define MO2_Pin GPIO_PIN_12
#define MO2_GPIO_Port GPIOB
#define MI2_Pin GPIO_PIN_13
#define MI2_GPIO_Port GPIOB
#define DCS_Pin GPIO_PIN_14
#define DCS_GPIO_Port GPIOB
#define CS2_Pin GPIO_PIN_15
#define CS2_GPIO_Port GPIOB
#define DP2_Pin GPIO_PIN_12
#define DP2_GPIO_Port GPIOD
#define DP3_Pin GPIO_PIN_13
#define DP3_GPIO_Port GPIOD
#define RST_Pin GPIO_PIN_6
#define RST_GPIO_Port GPIOC
#define Led_on_Pin GPIO_PIN_7
#define Led_on_GPIO_Port GPIOC
#define SN_buzzer_Pin GPIO_PIN_3
#define SN_buzzer_GPIO_Port GPIOD
#define DP4_Pin GPIO_PIN_4
#define DP4_GPIO_Port GPIOB
#define CS3_Pin GPIO_PIN_6
#define CS3_GPIO_Port GPIOB
#define MI3_Pin GPIO_PIN_7
#define MI3_GPIO_Port GPIOB
#define SC3_Pin GPIO_PIN_8
#define SC3_GPIO_Port GPIOB
#define MO3_Pin GPIO_PIN_9
#define MO3_GPIO_Port GPIOB
#define CMM_Pin GPIO_PIN_0
#define CMM_GPIO_Port GPIOE
#define ON_Pin GPIO_PIN_1
#define ON_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */
void main_task(void);
void init(void);
void kbtask();
void measure_task(void);
void rf_task();
extern IWDG_HandleTypeDef hiwdg;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
