
#include "tremo_it.h"
#include "stdio.h"
#include "tremo_rtc.h"
#include "timer.h"

extern void RadioOnDioIrq(void);
/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
    printf("NMI Exception\r\n");
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */

typedef struct __attribute__((packed)) ContextStateFrame
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t return_address;
    uint32_t xpsr;
} sContextStateFrame;

__attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile(
        "tst lr, #4 \n"
        "ite eq \n"
        "mrseq r0, msp \n"
        "mrsne r0, psp \n"
        "b my_fault_handler_c \n");
}

void my_fault_handler_c(uint32_t *sp)
{

    printf("Hard Fault\r\n");
    uint32_t cfsr = SCB->CFSR;
    uint32_t hfsr = SCB->HFSR;
    uint32_t mmfar = SCB->MMFAR;
    uint32_t bfar = SCB->BFAR;
    printf("SCB->CFSR   0x%08lx\r\n", cfsr);
    printf("SCB->HFSR   0x%08lx\r\n", hfsr);
    printf("SCB->MMFAR  0x%08lx\r\n", mmfar);
    printf("SCB->BFAR   0x%08lx\r\n", bfar);
    printf("\n");

    uint32_t r0 = sp[0];
    uint32_t r1 = sp[1];
    uint32_t r2 = sp[2];
    uint32_t r3 = sp[3];
    uint32_t r12 = sp[4];
    uint32_t lr = sp[5];
    uint32_t pc = sp[6];
    uint32_t psr = sp[7];
    printf("LAST STACK:\r\n");
    printf("  SP          0x%08lx\r\n", (uint32_t)sp);
    printf("  R0          0x%08lx\r\n", r0);
    printf("  R1          0x%08lx\r\n", r1);
    printf("  R2          0x%08lx\r\n", r2);
    printf("  R3          0x%08lx\r\n", r3);
    printf("  R12         0x%08lx\r\n", r12);
    printf("  LR          0x%08lx\r\n", lr);
    printf("  PC          0x%08lx\r\n", pc);
    printf("  PSR         0x%08lx\r\n", psr);

    while (1)
        ;
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    printf("MEM Exception\r\n");
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    printf("Busfault Exception\r\n");
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    printf("Usagefault Exception\r\n");
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles SVCall exception.
 * @param  None
 * @retval None
 */
void SVC_Handler(void)
{
    printf("SVC Exception\r\n");
}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */
void PendSV_Handler(void)
{
    printf("PendSV Exception\r\n");
}

/**
 * @brief  This function handles SysTick Handler.
 * @param  None
 * @retval None
 */
void SysTick_Handler(void)
{
}

/**
 * @brief  This function handles PWR Handler.
 * @param  None
 * @retval None
 */
void PWR_IRQHandler()
{

}

/******************************************************************************/
/*                 Tremo Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_cm4.S).                                               */
/******************************************************************************/

/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
{
}*/

void RTC_IRQHandler(void)
{
    uint8_t intr_stat;
    intr_stat = rtc_get_status(RTC_CYC_SR);

    if (intr_stat == true)
    {

        rtc_cyc_cmd(DISABLE);
        rtc_config_interrupt(RTC_CYC_IT, DISABLE); // disable
        rtc_set_status(RTC_CYC_SR, false);         // clear

        TimerIrqHandler();

        rtc_config_interrupt(RTC_CYC_IT, ENABLE); // enable
    }
}

void LORA_IRQHandler()
{
    RadioOnDioIrq();
}