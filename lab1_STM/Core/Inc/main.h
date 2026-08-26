/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

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
#define B_P1_Pin GPIO_PIN_0
#define B_P1_GPIO_Port GPIOA
#define B_P1_EXTI_IRQn EXTI0_IRQn
#define B_P2_Pin GPIO_PIN_1
#define B_P2_GPIO_Port GPIOA
#define B_P2_EXTI_IRQn EXTI1_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define B_Start_Pin GPIO_PIN_4
#define B_Start_GPIO_Port GPIOA
#define B_Start_EXTI_IRQn EXTI4_IRQn
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define L1_P2_Pin GPIO_PIN_6
#define L1_P2_GPIO_Port GPIOC
#define L2_P2_Pin GPIO_PIN_7
#define L2_P2_GPIO_Port GPIOC
#define L3_P2_Pin GPIO_PIN_8
#define L3_P2_GPIO_Port GPIOC
#define L4_P2_Pin GPIO_PIN_9
#define L4_P2_GPIO_Port GPIOC
#define L1_P1_Pin GPIO_PIN_10
#define L1_P1_GPIO_Port GPIOA
#define L2_P1_Pin GPIO_PIN_11
#define L2_P1_GPIO_Port GPIOA
#define L3_P1_Pin GPIO_PIN_12
#define L3_P1_GPIO_Port GPIOA
#define L4_P1_Pin GPIO_PIN_13
#define L4_P1_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
