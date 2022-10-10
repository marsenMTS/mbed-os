#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "max32670.h"
#include "gcr_regs.h"
#include "mxc_sys.h"

#define XDOT_ERFO_FREQ      24000000;

void SystemCoreClockUpdate(void)
{
    uint32_t base_freq, div, clk_src;

    // Get the clock source and frequency
    clk_src = (MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_SYSCLK_SEL);
    switch (clk_src)
    {
        case MXC_S_GCR_CLKCTRL_SYSCLK_SEL_EXTCLK:
            base_freq = EXTCLK_FREQ;
            break;
        case MXC_S_GCR_CLKCTRL_SYSCLK_SEL_ERFO:
#if defined(TARGET_XDOT_MAX32670)
            base_freq = XDOT_ERFO_FREQ;
#else
            base_freq = ERFO_FREQ;
#endif
            break;
        case MXC_S_GCR_CLKCTRL_SYSCLK_SEL_INRO:
            base_freq = INRO_FREQ;
            break;
        case MXC_S_GCR_CLKCTRL_SYSCLK_SEL_IPO:
        base_freq = IPO_FREQ;
            break;
        case MXC_S_GCR_CLKCTRL_SYSCLK_SEL_IBRO:
        base_freq = IBRO_FREQ;
            break;
        case MXC_S_GCR_CLKCTRL_SYSCLK_SEL_ERTCO:
            base_freq = ERTCO_FREQ;
            break;
        default:
            // Codes 001 and 111 are reserved.
            // This code should never execute, however, initialize to safe value.
            base_freq = HIRC_FREQ;
            break;
    }
    // Get the clock divider
    if (clk_src == MXC_S_GCR_CLKCTRL_SYSCLK_SEL_IPO)
    {
        base_freq = base_freq >> ((MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_IPO_DIV)>> MXC_F_GCR_CLKCTRL_IPO_DIV_POS);
    }
    div = (MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_SYSCLK_DIV) >> MXC_F_GCR_CLKCTRL_SYSCLK_DIV_POS;

    SystemCoreClock = base_freq >> div;
}

int PreInit(void)
{
    /*
     *  Switch to ERFO here instead of SystemInit
     */
    MXC_SYS_Clock_Select(MXC_SYS_CLOCK_ERFO);
    return 0;
}

void SystemInit(void)
{
    /* Make sure interrupts are enabled. */
    __enable_irq();

#if (__FPU_PRESENT == 1)
    /* Enable FPU on Cortex-M4, which occupies coprocessor slots 10 & 11 */
    /* Grant full access, per "Table B3-24 CPACR bit assignments". */
    /* DDI0403D "ARMv7-M Architecture Reference Manual" */
    SCB->CPACR |= SCB_CPACR_CP10_Msk | SCB_CPACR_CP11_Msk;
    __DSB();
    __ISB();
#endif

    /* Change system clock source to the main high-speed clock 
     * This is where the SDK switches to IPO. Switching to ERFO
     * here works, but each program execution needs a POR, same
     * with flashing the device. Could be a daplink issue, but
     * switching to ERFO in PreInit solves for this 
     */
    SystemCoreClockUpdate();

    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_GPIO0); 
    MXC_SYS_ClockEnable(MXC_SYS_PERIPH_CLOCK_GPIO1); 
}