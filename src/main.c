/* Copyright 2019, 2023, 2025 NXP */
/* License: BSD 3-clause
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

/*
 * main implementation: use this 'C' sample to create your own application
 *
 */
#include "S32K144.h"
#include "clock_mode.h"
#include "lptmr.h"
#include "device_registers.h"
#include <stdint.h>

/* LED pins from your PCB schematic */
#define LED1_PIN       9U     /* PTE9  - GPIO-3  */
#define LED2_PIN       8U     /* PTE8  - GPIO-4  */
#define LED3_PIN       11U    /* PTA11 - GPIO-10 */
#define LED4_PIN       13U    /* PTA13 - GPIO-8  */
#define LED5_PIN       12U    /* PTA12 - GPIO-9  */

#define LED1_MASK      (1UL << LED1_PIN)
#define LED2_MASK      (1UL << LED2_PIN)
#define LED3_MASK      (1UL << LED3_PIN)
#define LED4_MASK      (1UL << LED4_PIN)
#define LED5_MASK      (1UL << LED5_PIN)

#define LED_COUNT      5U
#define LED_DELAY_TICKS 50U

volatile uint32_t timer_count = 0U;
volatile uint8_t current_led = 0U;

/* Disable watchdog */
void WDOG_disable(void)
{
    IP_WDOG->CNT   = 0xD928C520U;
    IP_WDOG->TOVAL = 0x0000FFFFU;
    IP_WDOG->CS    = 0x00002100U;
}

/* Initialize the five GPIO pins */
void PORT_init(void)
{
    /* Enable PORTA and PORTE clocks */
    IP_PCC->PCCn[PCC_PORTA_INDEX] = PCC_PCCn_CGC_MASK;
    IP_PCC->PCCn[PCC_PORTE_INDEX] = PCC_PCCn_CGC_MASK;

    /* Select GPIO function */
    IP_PORTA->PCR[LED3_PIN] = PORT_PCR_MUX(1U);
    IP_PORTA->PCR[LED4_PIN] = PORT_PCR_MUX(1U);
    IP_PORTA->PCR[LED5_PIN] = PORT_PCR_MUX(1U);

    IP_PORTE->PCR[LED1_PIN] = PORT_PCR_MUX(1U);
    IP_PORTE->PCR[LED2_PIN] = PORT_PCR_MUX(1U);

    /* Configure LED pins as outputs */
    IP_PTA->PDDR |= LED3_MASK | LED4_MASK | LED5_MASK;
    IP_PTE->PDDR |= LED1_MASK | LED2_MASK;

    /* All LEDs OFF: active-low connection */
    IP_PTE->PSOR = LED1_MASK | LED2_MASK;
    IP_PTA->PSOR = LED3_MASK | LED4_MASK | LED5_MASK;
}

/* Turn all LEDs OFF */
void LEDs_AllOff(void)
{
    IP_PTE->PSOR = LED1_MASK | LED2_MASK;
    IP_PTA->PSOR = LED3_MASK | LED4_MASK | LED5_MASK;
}

/* Select one LED */
void LED_Select(uint8_t led)
{
    LEDs_AllOff();

    switch (led)
    {
        case 0U:
            IP_PTE->PCOR = LED1_MASK;
            break;

        case 1U:
            IP_PTE->PCOR = LED2_MASK;
            break;

        case 2U:
            IP_PTA->PCOR = LED3_MASK;
            break;

        case 3U:
            IP_PTA->PCOR = LED4_MASK;
            break;

        case 4U:
            IP_PTA->PCOR = LED5_MASK;
            break;

        default:
            LEDs_AllOff();
            break;
    }
}

/* Enable an interrupt using NVIC registers */
void Enable_Interrupt(uint8_t vector_number)
{
    uint32_t index = (uint32_t)vector_number >> 5U;
    uint32_t mask =
        1UL << ((uint32_t)vector_number & 0x1FU);

    volatile uint32_t *nvic_icpr =
        (volatile uint32_t *)0xE000E280UL;

    volatile uint32_t *nvic_iser =
        (volatile uint32_t *)0xE000E100UL;

    nvic_icpr[index] = mask;
    nvic_iser[index] = mask;
}

/* LPTMR0 interrupt handler */
void LPTMR0_IRQHandler(void)
{
    /* Clear timer compare flag while keeping timer enabled */
    IP_LPTMR0->CSR =
        LPTMR_CSR_TCF_MASK |
        LPTMR_CSR_TIE_MASK |
        LPTMR_CSR_TEN_MASK;

    timer_count++;

    if (timer_count >= LED_DELAY_TICKS)
    {
        timer_count = 0U;

        current_led++;

        if (current_led >= LED_COUNT)
        {
            current_led = 0U;
        }

        LED_Select(current_led);
    }
}

int main(void)
{
    WDOG_disable();

    /* Initialize clocks using your existing clock functions */
    SOSC_init_8MHz();
    SPLL_init_160MHz();
    NormalRUNmode_80MHz();

    /* Initialize LEDs */
    PORT_init();

    current_led = 0U;
    LED_Select(current_led);

    /* Initialize LPTMR0 */
    LPTMR_init();

    /* Enable LPTMR0 interrupt */
    Enable_Interrupt((uint8_t)LPTMR0_IRQn);

    /* Main loop */
    while (1)
    {
        /* LED sequence runs from the timer interrupt */
    }
}
