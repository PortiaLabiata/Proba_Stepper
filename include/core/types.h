#pragma once

/* Global flag options */

#define SET (uint8_t)1
#define RESET (uint8_t)0
#define MAX_TIMEOUT -1

#define CLOCKWISE (uint8_t)0
#define COUNTERCLOCKWISE (uint8_t)1

/* Bitmasks and indices for Modbus registers */

#define INDEX_HOLD_CMD   0
#define INDEX_HOLD_STEPS 1
#define INDEX_HOLD_MODE  2
#define INDEX_HOLD_SPEED 3

#define INDEX_INP_ERRC  0

#define STP_MODE_FULL1  0
#define STP_MODE_FULL2  1
#define STP_MODE_HALF   2
#define STP_DIR_CLOCK   CLOCKWISE
#define STP_DIR_CCLOCK  COUNTERCLOCKWISE

/* Forward declarations */

typedef struct UART_Handle UART_Handle_t;
typedef struct Stepper_Handle Stepper_Handle_t;

/* Global typedefs */

/**
 * \struct
 * \brief Global context structure, there is meant to be only one object.
 */
typedef struct {
    UART_Handle_t *uart_handle;
    Stepper_Handle_t *stepper_handle;
} System_Context_t;

typedef enum {
    STEPPER_OK,
    STEPPER_ERROR_SOFT
} Stepper_Status_t;

typedef enum {
    UART_OK,
    UART_ERROR_SOFT
} UART_Status_t;

typedef enum {
    SYS_OK,
    SYS_ERROR
} System_Status_t;

typedef enum {
    CMD_NOCMD,
    CMD_ROTATE,
    CMD_HALT
} Command_t;

/* System context */

extern System_Context_t ctx;