#include "core/system.h"
#include "core/types.h"
#include "utils/service.h"

/* Constants */

#define N_PINS (uint8_t)4
#define MAX_STEPPERS 2

#define STEPPER_LEAVE_ON_NULL(__CONF__) if (__CONF__ == NULL) return STEPPER_ERROR_SOFT;

/* Typedefs */

typedef enum {
    STEPPER_MODE_FULLSTEP_1PHASE,
    STEPPER_MODE_FULLSTEP_2PHASE,
    STEPPER_MODE_HALFSTEP
} Stepper_Mode_t;

typedef enum {
    STEPPER_OK,
    STEPPER_ERROR_SOFT
} Stepper_Status_t;

typedef struct Stepper_Handle Stepper_Handle_t;

/* Functions */

Stepper_Handle_t *Stepper_Init(TIM_TypeDef *inst, const uint32_t *gpios, const uint8_t *config_1ph, const uint8_t *config_2ph, \
    const uint8_t *config_half);
Stepper_Status_t Stepper_Step(Stepper_Handle_t *stp, uint8_t dir);
Stepper_Status_t Stepper_SetMode(Stepper_Handle_t *stp, Stepper_Mode_t mode);

Stepper_Status_t Stepper_Halt(Stepper_Handle_t *stp, uint8_t hold);
Stepper_Status_t Stepper_Rotate(Stepper_Handle_t *stp, uint32_t steps, uint8_t dir, uint32_t del);
Stepper_Status_t Stepper_Rotate_IT(Stepper_Handle_t *stp, uint32_t steps, uint8_t dir, uint32_t del);
Stepper_Status_t Stepper_Halt_IT(Stepper_Handle_t *stp, uint8_t hold);

/* ISRs */

uint8_t TIM_UEV_Callback(System_Context_t *ctx);
