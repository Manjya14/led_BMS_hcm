/*
 * lptmr.c
 *
 *  Created on: 26-Oct-2026
 *      Author: Manjula
 */
//#include "device_registers.h"
//#include "lptmr.h"
//
//void LPTMR_init(void)
//{
//	IP_PCC->PCCn[PCC_LPTMR0_INDEX] = PCC_PCCn_CGC_MASK;	/* Enable LPTMR Clock 		*/
//    IP_LPTMR0->PSR |= LPTMR_PSR_PCS(0b01)                  /* LPTMR clk src: 1KHz LPO */
//                  |LPTMR_PSR_PBYP_MASK;               	/* Bypass Prescaler 		*/
//    IP_LPTMR0->CMR = 9;                                  /* 10 ms interrupt 		*/
//    IP_LPTMR0->CSR |= LPTMR_CSR_TIE_MASK; 					/* Timer interrupt enabled */
//    IP_LPTMR0->CSR |= LPTMR_CSR_TEN_MASK;                  /* Enable Timer 			*/
//}

#include "S32K144.h"
#include "lptmr.h"

void LPTMR_init(void)
{
    /* Enable LPTMR0 peripheral clock */
    IP_PCC->PCCn[PCC_LPTMR0_INDEX] = 0U;
    IP_PCC->PCCn[PCC_LPTMR0_INDEX] =
        PCC_PCCn_PCS(1U) |
        PCC_PCCn_CGC_MASK;

    /* Disable timer for configuration */
    IP_LPTMR0->CSR = 0U;

    /* Select 1 kHz LPO, bypass prescaler */
    IP_LPTMR0->PSR =
        LPTMR_PSR_PCS(1U) |
        LPTMR_PSR_PBYP_MASK;

    /* Compare value 9 = 10 clock cycles */
    IP_LPTMR0->CMR = 9U;

    /* Clear pending compare flag */
    IP_LPTMR0->CSR = LPTMR_CSR_TCF_MASK;

    /* Enable interrupt and start timer */
    IP_LPTMR0->CSR =
        LPTMR_CSR_TIE_MASK |
        LPTMR_CSR_TEN_MASK;
}
