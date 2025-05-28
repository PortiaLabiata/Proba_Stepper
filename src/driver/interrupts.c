#include "driver/interrupts.h"
#include "driver/stepper.h"

void SysTick_Handler(void) {
    _current_ticks++;
}

void TIM2_IRQHandler(void) {
    TIM2->SR &= ~(TIM_SR_UIF); // Reset the timer status
    Stepper_TIMCallback();
}