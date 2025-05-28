#include "main.h"
#include "utils/service.h"
#include "driver/interrupts.h"
#include "driver/uart.h"
#include "driver/stepper.h"
#include "driver/iwdg.h"
#include "app/mb_logic.h"

#include "mb.h"
#include "port.h"

static System_Status_t eSystemPoll(MB_Proxy_t *proxy);

const uint8_t wave[4] = {
    0b1000,
    0b0010,
    0b0100,
    0b0001
};

const uint8_t wave_full_phase[4] = {
    0b1010,
    0b0110,
    0b0101,
    0b1001
};

const uint32_t gpios[4] = {GPIO_ODR_ODR6, GPIO_ODR_ODR7, GPIO_ODR_ODR8, GPIO_ODR_ODR9};
volatile uint8_t buffer[10];

int main(void) {
    Clock_Config();
    GPIO_Config();
    TIM2_Config();
    TIM3_Config();

#ifdef STARTUP_BLINK_ENABLE
    STARTUP_BLINK();
#endif

    IWDG_Config();

    Stepper_Handle_t *stp = Stepper_Create();
    Stepper_Init(stp, GPIOA, GPIOA, GPIOA, GPIO_ODR_ODR1, GPIO_ODR_ODR2, GPIO_ODR_ODR3);
    Stepper_Enable();
    UART_Handle_t *hnd = UART_Init(USART1);
    MB_Proxy_t *proxy = MB_Proxy_Init();
    Context_Init(&ctx, hnd, stp);

    eMBErrorCode eStatus; // For later
    eStatus = eMBInit(MB_RTU, 1, 0, MB_BAUD_RATE, MB_PAR_EVEN);
    eStatus = eMBEnable();

    //Stepper_SetMode(ctx.stepper_handle, STEPPER_MODE_FULLSTEP_1PHASE);

    while (1) {
        (void)eMBPoll();
        (void)eSystemPoll(proxy);
        IWDG_RELOAD();
    }

}

/**
 * \todo Add error handling.
 */
static System_Status_t eSystemPoll(MB_Proxy_t *proxy) {
    MB_Proxy_Unmarshall(proxy);
    System_Status_t status = SYS_OK;
    uint32_t speed = 0;
    switch (MB_Proxy_AccessCmd(proxy)) {
        case CMD_NOCMD:
            break;

        case CMD_ROTATE:
            speed = MB_Proxy_AccessSpeed(proxy);
            if (speed < 10 || speed > 50) {
                MB_Proxy_SetCmd(proxy, CMD_NOCMD);
                status = SYS_ERROR_ILLVALUE;
                break;
            }

            //Stepper_SetMode(ctx.stepper_handle, MB_Proxy_AccessMode(proxy));
            Stepper_Rotate_IT(ctx.stepper_handle, MB_Proxy_AccessSteps(proxy), \
                MB_Proxy_AccessDir(proxy));
            break;

        case CMD_HALT:
            Stepper_Halt_IT(ctx.stepper_handle);
            break;
        
        case CMD_INVALID:
            status = SYS_ERROR_ILLVALUE;
            break;
        
        default:
            status = SYS_ERROR;
            break;
    }
    MB_Proxy_SetCmd(proxy, CMD_NOCMD);
    MB_Proxy_Marshall(proxy);
    return status;
} 
