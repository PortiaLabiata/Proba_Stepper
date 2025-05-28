#include "driver/stepper.h"
#include "core/system.h"
#include "core/types.h"
#include "core/prefs.h"

/* Typedefs */

struct Stepper_Handle {
    GPIO_TypeDef *en_port;
    uint32_t en_msk;
    GPIO_TypeDef *dir_port;
    uint32_t dir_msk;
    GPIO_TypeDef *ms1_port;
    uint32_t ms1_msk;
    uint32_t steps_left;
    uint8_t enabled;
};

/* Global variables */

static Stepper_Handle_t _stepper_pool[STP_POOL_SIZE];
static uint8_t _stepper_idx = 0;

/* Constructor/destructor */

Stepper_Handle_t *Stepper_Create(void) {
    if (_stepper_idx >= STP_POOL_SIZE) {
        return NULL;
    }
    return &_stepper_pool[_stepper_idx++];
}

/* Control functions */

Stepper_Error_t Stepper_Init(Stepper_Handle_t *handle, GPIO_TypeDef *en_port, GPIO_TypeDef *dir_port,\
    GPIO_TypeDef *ms1_port, uint32_t en_msk, uint32_t dir_msk, uint32_t ms1_msk) {
    if (handle == NULL) {
        return STP_ERR_NULLPTR;
    }
    handle->dir_port = dir_port;
    handle->dir_msk = dir_msk;

    handle->en_port = en_port;
    handle->en_msk = en_msk;

    handle->ms1_port = ms1_port;
    handle->ms1_msk = ms1_msk;
    return STP_OK;
}

void Stepper_Enable(void) {
    // PWM mode 2
    STP_TIM_INSTANCE->CCMR1 |= (TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
    STP_TIM_INSTANCE->CCER |= TIM_CCER_CC1E;
    STP_TIM_INSTANCE->PSC = SystemCoreClock / 1000 - 1; // One timer tick = 1 ms
    STP_TIM_INSTANCE->ARR = 10 - 1;
    STP_TIM_INSTANCE->CCR1 = 5;

    STP_TIM_INSTANCE->DIER |= TIM_DIER_UIE;
    STP_TIM_INSTANCE->CR1 &= ~(TIM_CR1_CKD_Msk);
    STP_TIM_INSTANCE->EGR |= TIM_EGR_UG;
    STP_TIM_INSTANCE->CR1 |= TIM_CR1_CEN;
}

Stepper_Error_t Stepper_SetPeriod(uint16_t period_ms) {
    if (period_ms < 7 || period_ms > 50) {
        return STP_ERR_ILLVAL;
    }
    STP_TIM_INSTANCE->CCR1 = period_ms;
    return STP_OK;
}

void Stepper_SetSteps(Stepper_Handle_t *handle, uint16_t steps) {
    handle->steps_left = steps;
}

Stepper_Error_t Stepper_SetMode(Stepper_Handle_t *handle, Stepper_Mode_t mode) {
    switch (mode) {
        case STP_MODE_1:
            STP_SETMODE_1(handle);
            break;
        case STP_MODE_2:
            STP_SETMODE_2(handle);
            break;
        default:
            return STP_ERR_ILLVAL;
    }
    return STP_OK;
}

/* Rotation */

uint8_t Stepper_IsEnabled(Stepper_Handle_t *handle) {
    return handle->enabled;
}

Stepper_Error_t Stepper_Rotate_IT(Stepper_Handle_t *handle, uint16_t steps, \
     Stepper_Dir_t direc) {
    if (handle == NULL) {
        return STP_ERR_NULLPTR;
    }

    switch (direc) {
        case STP_DIR_CLOCK:
            STP_SETDIR_CLOCK(handle);
            break;
        case STP_DIR_COUNTER:
            STP_SETDIR_COUNTER(handle);
            break;
        default:
            return STP_ERR_ILLVAL;
    }
    Stepper_SetSteps(handle, steps);
    STP_ENABLE(handle);
    return STP_OK;
}

Stepper_Error_t Stepper_Halt_IT(Stepper_Handle_t *handle) {
    if (handle == NULL) {
        return STP_ERR_NULLPTR;
    }
    Stepper_SetSteps(handle, 0);
    STP_DISABLE(handle);
    return STP_OK;
}

/* Callbacks */

void Stepper_TIMCallback(void) {
    for (Stepper_Handle_t *handle = _stepper_pool; handle - _stepper_pool < _stepper_idx; handle++) {
        if (--handle->steps_left <= 0 && Stepper_IsEnabled(handle)) {
            STP_DISABLE(handle);
        }
    }
}