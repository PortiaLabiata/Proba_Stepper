#include "driver/stepper.h"

/* Typedefs */

/**
 * \todo Microoptimization: switch from array to single variables. It will make time
 * of reading a value just a little bit shorter.
 */
struct Stepper_Handle {
    const uint32_t *gpios;
    const uint8_t *config;
    TIM_TypeDef *instance;
    volatile uint8_t config_idx;
    uint8_t direc;
    volatile uint32_t steps_left;
    uint8_t mode;

    const uint8_t *config_1ph;
    const uint8_t *config_2ph;
    const uint8_t *config_half;
};

/* Handle pool */

static Stepper_Handle_t _stepper_pool[MAX_STEPPERS];
static uint8_t _n_steppers;

/* Functions */

/**
 * \brief Initializes stepper object from the global pool.
 * \param[in] gpios Array of GPIO pins used for stepper.
 * \param[in] configs Array of sequential configurations of stepper GPIO pins.
 * \returns Pointer to a stepper object. Unsafe, btw.
 */
Stepper_Handle_t *Stepper_Init(const uint32_t *gpios, const uint8_t *config_1ph, const uint8_t *config_2ph, \
    const uint8_t *config_half) {
    if (_n_steppers >= MAX_STEPPERS) return NULL;
    
    Stepper_Handle_t *stp = &_stepper_pool[_n_steppers++];
    stp->gpios = gpios;
    stp->config = config_1ph;

    stp->config_1ph = config_1ph;
    stp->config_2ph = config_2ph;
    stp->config_half = config_half;

    stp->config_idx = 0;
    stp->mode = STEPPER_MODE_FULLSTEP_1PHASE;

    GPIOB->ODR &= ~(gpios[0] | gpios[1] | gpios[2] | gpios[3]);
    return stp;
}

/**
 * \brief Sets the stepper mode, either full step 1 phase, full step 2 phase of half step.
 * \param[in] stp Stepper handle.
 * \param[in] mode Stepper mode, either STEPPER_MODE_FULLSTEP_1PHASE, STEPPER_MODE_FULLSTEP_2PHASE,
 * or STEPPER_MODE_HALFSTEP.
 * \returns Operation status.
 */
Stepper_Status_t Stepper_SetMode(Stepper_Handle_t *stp, Stepper_Mode_t mode) {
    switch (mode) {
        case STEPPER_MODE_FULLSTEP_1PHASE:
            STEPPER_LEAVE_ON_NULL(stp->config_1ph);
            stp->config = stp->config_1ph;
            break;
        case STEPPER_MODE_FULLSTEP_2PHASE:
            STEPPER_LEAVE_ON_NULL(stp->config_2ph);
            stp->config = stp->config_2ph;
            break;
        case STEPPER_MODE_HALFSTEP:
            STEPPER_LEAVE_ON_NULL(stp->config_half);
            stp->config = stp->config_half;
            break;
        default:
            return STEPPER_ERROR_SOFT;
    }
    stp->mode = mode;
    return STEPPER_OK;
}

/**
 * \brief Preforms a single step.
 * \param[in] stp Stepper handle.
 * \param[in] dir Direction, can be either CLOCKWISE or COUNTERCLOCKWISE.
 * \returns Operation status.
 */
Stepper_Status_t Stepper_Step(Stepper_Handle_t *stp, uint8_t dir) {
    uint8_t curr_config = stp->config[stp->config_idx];
    GPIOB->ODR &= ~(stp->gpios[0] | stp->gpios[1] | stp->gpios[2] | stp->gpios[3]);

    if (curr_config & 0b0001) GPIOB->ODR |= stp->gpios[3];
    if (curr_config & 0b0010) GPIOB->ODR |= stp->gpios[2];
    if (curr_config & 0b0100) GPIOB->ODR |= stp->gpios[1];
    if (curr_config & 0b1000) GPIOB->ODR |= stp->gpios[0];

    if (dir == CLOCKWISE) {
        stp->config_idx = (stp->config_idx + 1) % N_PINS;
    } else if (dir == COUNTERCLOCKWISE) {
        stp->config_idx = (stp->config_idx - 1 + N_PINS) % N_PINS;
    } else {
        return STEPPER_ERROR_SOFT; // In case something went horribly wrong.
    }
    return STEPPER_OK;
}

/**
 * \brief Halts a stepper motor and either leaves it energized 
 * (huge current consumption!) or not.
 * \param[in] stp Stepper handle.
 * \param[in] hold Leave the coils energized or not, either SET or RESET.
 * \returns Operation status.
 */
Stepper_Status_t Stepper_Halt(Stepper_Handle_t *stp, uint8_t hold) {
    if (!hold) {
        GPIOB->ODR &= ~(stp->gpios[0] | stp->gpios[1] | stp->gpios[2] | stp->gpios[3]);
        return STEPPER_OK;
    } else {
        return STEPPER_OK;
    }
}


/**
 * \brief Performs a blocking stepper revolution. Currently only full step mode supported.
 * Minimum delay is 10 ms, technically there is no maximum delay, but at 100 ms the
 * current cunsomption increases to 300 mA, so I would say a max of 50-70 ms is recommended.
 * \param[in] stp Stepper handle.
 * \param[in] steps Number of steps performed.
 * \param[in] dir Direction, can be either CLOCKWISE or COUNTERCLOCKWISE.
 * \param[in] del Delay between steps.
 * \returns Operation status.
 * \todo Add error handling.
 */
Stepper_Status_t Stepper_Rotate(Stepper_Handle_t *stp, uint32_t steps, uint8_t dir, uint32_t del) {
    for (int i = 0; i < steps; i++) {
        Stepper_Step(stp, dir);
        delay(del);
    }

    return STEPPER_OK;
}

/**
 * \brief Performs a non-blocking stepper revolution. Currently only full step mode supported.
 * Minimum delay is 10 ms, technically there is no maximum delay, but at 100 ms the
 * current cunsomption increases to 300 mA, so I would say a max of 50-70 ms is recommended.
 * \param[in] stp Stepper handle.
 * \param[in] steps Number of steps performed.
 * \param[in] dir Direction, can be either CLOCKWISE or COUNTERCLOCKWISE.
 * \param[in] del Delay between steps.
 * \returns Operation status.
 * \todo Add error handling.
 */
Stepper_Status_t Stepper_Rotate_IT(Stepper_Handle_t *stp, uint32_t steps, uint8_t dir, uint32_t del) {
    stp->steps_left = steps;
    stp->direc = dir;

    TIM2->PSC = PCLK1_FREQ / 1000 - 1;
    TIM2->ARR = del - 1;
    TIM2->EGR |= TIM_EGR_UG;
    TIM2->CR1 |= TIM_CR1_CEN;
    return STEPPER_OK;
}

/**
 * \brief Halts stepper in non-blocking mode analogous to blocking case.
 * \param[in] stp Stepper handle.
 * \param[out] hold Leave the coild energized or not, either SET or RESET.
 * \returns Operation status.
 */
Stepper_Status_t Stepper_Halt_IT(Stepper_Handle_t *stp, uint8_t hold) {
    stp->steps_left = 0;
    return Stepper_Halt(stp, hold);
}

/* Callbacks */

/**
 * \brief Callback for timer UEV, steps the stepper from the system context.
 * \param[in] ctx System context.
 * \returns Operation status, either SET or RESET. The IRS will decide if it's time to
 * stop the timer by the return value, so low-level and driver-level modules are separated.
 */
uint8_t TIM_UEV_Callback(System_Context_t *ctx) {
    Stepper_Handle_t *handle = ctx->stepper_handle;
    if (handle->steps_left > 0) {
        Stepper_Step(handle, handle->direc);
        handle->steps_left--;
        return SET;
    } else {
        Stepper_Halt(handle, RESET);
        return RESET;
    }
}

