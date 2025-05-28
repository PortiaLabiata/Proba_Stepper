#pragma once
#include <stm32f103x6.h>

/* Constants */

#define STP_POOL_SIZE       4
#define STP_TIM_INSTANCE    TIM2
#define TIM_IRQ(__TIM__)    __TIM__##_IRQn

/* Macros */

#define STP_ENABLE(__HANDLE__) do { \
    __HANDLE__->en_port->ODR |= __HANDLE__->en_msk; \
    __HANDLE__->enabled = SET;} while (0)

#define STP_DISABLE(__HANDLE__) do { \
    __HANDLE__->en_port->ODR &= ~(__HANDLE__->en_msk); \
    __HANDLE__->enabled = RESET;} while (0)

#define STP_SETMODE_1(__HANDLE__) do { \
    __HANDLE__->ms1_port->ODR &= ~(__HANDLE__->ms1_msk);} while (0)

#define STP_SETMODE_2(__HANDLE__) do { \
    __HANDLE__->ms1_port->ODR |= __HANDLE__->ms1_msk;} while (0)

#define STP_SETDIR_CLOCK(__HANDLE__) __HANDLE__->dir_port->ODR |= __HANDLE__->dir_msk
#define STP_SETDIR_COUNTER(__HANDLE__) __HANDLE__->dir_port->ODR &= ~(__HANDLE__->dir_msk)

/* Typedefs */

typedef enum {
    STP_OK,
    STP_ERR_ILLVAL,
    STP_ERR_NULLPTR
} Stepper_Error_t;

typedef enum {
    STP_MODE_1,
    STP_MODE_2,
    STP_MODE_4,
    STP_MODE_8,
    STP_MODE_16
} Stepper_Mode_t;

typedef enum {
    STP_DIR_CLOCK,
    STP_DIR_COUNTER
} Stepper_Dir_t;

typedef struct Stepper_Handle Stepper_Handle_t;

/* Prorotypes */

Stepper_Handle_t *Stepper_Create(void);
Stepper_Error_t Stepper_Init(Stepper_Handle_t *handle, GPIO_TypeDef *en_port, GPIO_TypeDef *dir_port,\
    GPIO_TypeDef *ms1_port, uint32_t en_msk, uint32_t dir_msk, uint32_t ms1_msk);
void Stepper_Enable(void);
Stepper_Error_t Stepper_SetPeriod(uint16_t period_ms);
void Stepper_SetSteps(Stepper_Handle_t *handle, uint16_t steps);
Stepper_Error_t Stepper_SetMode(Stepper_Handle_t *handle, Stepper_Mode_t mode);
uint8_t Stepper_IsEnabled(Stepper_Handle_t *handle);

Stepper_Error_t Stepper_Rotate_IT(Stepper_Handle_t *handle, uint16_t steps, \
    Stepper_Dir_t direc);
Stepper_Error_t Stepper_Halt_IT(Stepper_Handle_t *handle);

void Stepper_TIMCallback(void);