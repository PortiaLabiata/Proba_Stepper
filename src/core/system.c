#include "core/system.h"

/* SysTick configuration */

volatile uint32_t _current_ticks; // Current SysTick ticks, counted from startup.

/* Clock constants, will be calculated later */

uint32_t _sysclk_freq = 0;
uint32_t _ahb_freq = 0;
uint32_t _pclk2_freq = 0;
uint32_t _pclk1_freq = 0;

/* System information functions */

/**
 * \brief Calculates SYSCLK frequency based on RCC registers content. Is supposed to 
 * only run once at the start of the program.
 * \returns Frequency in hertz.
 */
static uint32_t Get_SYSCLK_Freq(void) {
    uint32_t pmul;
    switch (RCC->CFGR & (3UL << RCC_CFGR_SWS_Pos)) {
        case RCC_CFGR_SWS_HSI:
            return HSI_FREQ;
        case RCC_CFGR_SWS_HSE:
            return HSE_FREQ;
        case RCC_CFGR_SWS_PLL:
            pmul = (RCC->CFGR >> RCC_CFGR_PLLMULL_Pos) & 0b1111;
            return (pmul + 2)*HSI_FREQ/2;
        default:
            return 0; // Something went horribly wrong
    }
}

/**
 * \brief Calculates AHB frequency based on RCC registers content. Is supposed to 
 * only run once at the start of the program.
 * \returns Frequency in hertz.
 */
static uint32_t Get_AHB_Freq(void) {
    switch (RCC->CFGR & (0b1111 << RCC_CFGR_HPRE_Pos)) {
        case RCC_CFGR_HPRE_DIV1:
            return SYSCLK_FREQ / 1;
        case RCC_CFGR_HPRE_DIV2:
            return SYSCLK_FREQ / 2;
        case RCC_CFGR_HPRE_DIV4:
            return SYSCLK_FREQ / 4;
        case RCC_CFGR_HPRE_DIV8:
            return SYSCLK_FREQ / 8;
        case RCC_CFGR_HPRE_DIV16:
            return SYSCLK_FREQ / 16;
        case RCC_CFGR_HPRE_DIV64:
            return SYSCLK_FREQ / 64;
        case RCC_CFGR_HPRE_DIV128:
            return SYSCLK_FREQ / 128;
        case RCC_CFGR_HPRE_DIV256:
            return SYSCLK_FREQ / 256;
        case RCC_CFGR_HPRE_DIV512:
            return SYSCLK_FREQ / 512;
        default:
            return 0; // Something went horribly wrong
    }
}

/**
 * \brief Calculates PCLK2 frequency based on RCC registers content. Is supposed to 
 * only run once at the start of the program.
 * \returns Frequency in hertz.
 */
static uint32_t Get_PCLK2_Freq(void) {
    switch (RCC->CFGR & (0b111 << RCC_CFGR_PPRE2_Pos)) {
        case RCC_CFGR_PPRE2_DIV1:
            return AHB_FREQ / 1;
        case RCC_CFGR_PPRE2_DIV2:
            return AHB_FREQ / 2;
        case RCC_CFGR_PPRE2_DIV4:
            return AHB_FREQ / 4;
        case RCC_CFGR_PPRE2_DIV8:
            return AHB_FREQ / 8;
        case RCC_CFGR_PPRE2_DIV16:
            return AHB_FREQ / 16;
        default:
            return 0;
    }
}

/**
 * \brief Calculates PCLK1 frequency based on RCC registers content. Is supposed to 
 * only run once at the start of the program.
 * \returns Frequency in hertz.
 */
static uint32_t Get_PCLK1_Freq(void) {
    switch (RCC->CFGR & (0b111 << RCC_CFGR_PPRE1_Pos)) {
        case RCC_CFGR_PPRE1_DIV1:
            return AHB_FREQ / 1;
        case RCC_CFGR_PPRE1_DIV2:
            return AHB_FREQ / 2;
        case RCC_CFGR_PPRE1_DIV4:
            return AHB_FREQ / 4;
        case RCC_CFGR_PPRE1_DIV8:
            return AHB_FREQ / 8;
        case RCC_CFGR_PPRE1_DIV16:
            return AHB_FREQ / 16;
        default:
            return 0;
    }
}


/**
 * \brief Gets current SysTick tick from the global variable.
 * \returns Current tick.
 */
uint32_t Get_CurrentTick(void) {
    return _current_ticks;
}

/**
 * \brief Gets current TIM DTS frequency.
 * \param[in] tim Timer structure.
 * \returns Frequency, Hz.
 */
uint32_t Get_TIM_Div(TIM_TypeDef *tim) {
    const uint32_t fclk = (tim == TIM1) ? PCLK2_FREQ : PCLK1_FREQ;
    switch (tim->CR1 & TIM_CR1_CKD_Msk) {
        case 0:
            return fclk;
        case TIM_CR1_CKD_0:
            return fclk * 2;
        case TIM_CR1_CKD_1:
            return fclk * 4;
        default:
            return 0;
    }
}
/* Configuration functions */

/**
 * \brief Low-level configuration of RCC.
 */
void ClockConfig(void) {
    RCC->CR |= RCC_CR_HSION_Msk; // Starting HSI
    while (!(RCC->CR & RCC_CR_HSIRDY_Msk)) 
        __NOP();

    // Setting PLL source is unnecessary, since reset value is 0 and
    // it corresponds to HSI/2. f=48MHz
    // Settings: PLL as SYSCLK clock source, PLLMUL=12, ADC PSC=4.

    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV4 | RCC_CFGR_SW_PLL | RCC_CFGR_PLLMULL12;
    RCC->CR |= RCC_CR_PLLON; // Starting PLL
    while (!(RCC->CR & RCC_CR_PLLRDY)) 
        __NOP();
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) 
        __NOP();

    _sysclk_freq = Get_SYSCLK_Freq();
    _ahb_freq = Get_AHB_Freq();
    _pclk2_freq = Get_PCLK2_Freq();
    _pclk1_freq = Get_PCLK1_Freq();
    FLASH->ACR |= FLASH_ACR_LATENCY_2; // Enable flash latency

    NVIC_SetPriorityGrouping(4);
    SysTick_Config(AHB_FREQ / 1000); // Set SysTick to fire each ms
    NVIC_EnableIRQ(SysTick_IRQn); // Enable SysTick IRQ
}

/**
 * \brief Low-level configuration of GPIO, excluding those that UART uses.
 */
void GPIO_Config(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN_Msk; // Enable GPIOC clocking
    GPIOC->CRH |= (GPIO_MODE_OUTPUT << GPIO_CRH_MODE13_Pos); // Set mode to 2MHz
    GPIOC->CRH &= ~(GPIO_OUTPUT_PP << GPIO_CRH_CNF13_Pos); // Set mode to output PP

    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    GPIOB->CRL &= ~(GPIO_CRL_CNF6_Msk | GPIO_CRL_CNF7_Msk);
    GPIOB->CRH &= ~(GPIO_CRH_CNF8_Msk | GPIO_CRH_CNF9_Msk);

    GPIOB->CRL |= (GPIO_CRL_MODE6_1 | GPIO_CRL_MODE7_1);
    GPIOB->CRH |= (GPIO_CRH_MODE8_1 | GPIO_CRH_MODE9_1);
}

/**
 * \brief Low-level configuration of UART, including GPIO.
 */
void UART_Config(void) {
    RCC->APB2ENR |= (RCC_APB2ENR_USART1EN | RCC_APB2ENR_IOPAEN);

    // Configure PB6 (TX) as Alternate Function Push-Pull Output
    GPIOA->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9);
    GPIOA->CRH |= (GPIO_CRH_CNF9_1 | GPIO_CRH_MODE9_1);  // 50MHz output

    // Configure PB7 (RX) as Input Floating (no pull-up/pull-down)
    GPIOA->CRH &= ~(GPIO_CRH_CNF10 | GPIO_CRH_MODE10);
    GPIOA->CRH |= GPIO_CRH_CNF10_1;  // Input floating

    /* USART config */

    USART1->BRR = PCLK2_FREQ / UART_BAUD_RATE;
    USART1->CR1 |= (USART_CR1_TE | USART_CR1_RE | \
             USART_CR1_UE); // Enable UART, reciever and transmitter
    // No need to set M bit, word length=8 bit.

    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
}

/**
 * \brief Low-level configuration of TIM2.
 */
void TIM2_Config(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable clocking
    TIM2->DIER |= TIM_DIER_UIE; // Enable update event interrupt
    TIM2->CR1 &= ~(TIM_CR1_CKD_Msk);
    NVIC_SetPriority(TIM2_IRQn, 0);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/* System functions */

/**
 * \brief Busy-waiting loop for delays, based on SysTick ticks.
 * \param[in] ms Delay duration in ms.
 */
void delay(uint32_t ms) {
    uint32_t tick_start = Get_CurrentTick();
    while (Get_CurrentTick() - tick_start < ms) {
        __NOP();
    }
}