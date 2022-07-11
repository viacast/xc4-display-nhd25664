/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define L_1_R_Pin GPIO_PIN_12
#define L_1_R_GPIO_Port GPIOB
#define L_1_G_Pin GPIO_PIN_13
#define L_1_G_GPIO_Port GPIOB
#define L_2_R_Pin GPIO_PIN_14
#define L_2_R_GPIO_Port GPIOB
#define L_2_G_Pin GPIO_PIN_15
#define L_2_G_GPIO_Port GPIOB
#define L_3_R_Pin GPIO_PIN_9
#define L_3_R_GPIO_Port GPIOA
#define L_3_G_Pin GPIO_PIN_10
#define L_3_G_GPIO_Port GPIOA
#define L_4_R_Pin GPIO_PIN_15
#define L_4_R_GPIO_Port GPIOA
#define L_4_G_Pin GPIO_PIN_3
#define L_4_G_GPIO_Port GPIOB
#define SET_P_Pin GPIO_PIN_4
#define SET_P_GPIO_Port GPIOB
#define SET_P_EXTI_IRQn EXTI4_IRQn
#define RESET_P_Pin GPIO_PIN_5
#define RESET_P_GPIO_Port GPIOB
#define RESET_P_EXTI_IRQn EXTI9_5_IRQn
#define UP_P_Pin GPIO_PIN_6
#define UP_P_GPIO_Port GPIOB
#define UP_P_EXTI_IRQn EXTI9_5_IRQn
#define DOWN_P_Pin GPIO_PIN_7
#define DOWN_P_GPIO_Port GPIOB
#define DOWN_P_EXTI_IRQn EXTI9_5_IRQn
#define LEFT_P_Pin GPIO_PIN_8
#define LEFT_P_GPIO_Port GPIOB
#define LEFT_P_EXTI_IRQn EXTI9_5_IRQn
#define RIGHT_P_Pin GPIO_PIN_9
#define RIGHT_P_GPIO_Port GPIOB
#define RIGHT_P_EXTI_IRQn EXTI9_5_IRQn
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
