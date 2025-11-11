#include "stm32f1xx.h"
#include "FreeRTOS.h"
#include "core_cm3.h"
#include "lp_rtc_freertos.h"
#include "main.h"
#include "task.h"

const TickType_t xMaximumPossibleSuppressedTicks = 36000000UL; //1 час для тика 1мс
const uint32_t ulStoppedTimerCompensation = 0;
const int32_t RTC_FREQENCY = 32768;
const int32_t SYSTICK_PERIOD = 8000UL;

#define portNVIC_SYSTICK_CTRL_REG (*((volatile uint32_t *)0xe000e010))
#define portNVIC_SYSTICK_LOAD_REG (*((volatile uint32_t *)0xe000e014))
#define portNVIC_SYSTICK_CURRENT_VALUE_REG (*((volatile uint32_t *)0xe000e018))
#define portNVIC_SYSPRI2_REG (*((volatile uint32_t *)0xe000ed20))
/* ...then bits in the registers. */
#define portNVIC_SYSTICK_INT_BIT (1UL << 1UL)
#define portNVIC_SYSTICK_ENABLE_BIT (1UL << 0UL)
#define portNVIC_SYSTICK_COUNT_FLAG_BIT (1UL << 16UL)
#define portNVIC_PENDSVCLEAR_BIT (1UL << 27UL)
#define portNVIC_PEND_SYSTICK_CLEAR_BIT (1UL << 25UL)

#define portNVIC_PENDSV_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 16UL)
#define portNVIC_SYSTICK_PRI (((uint32_t)configKERNEL_INTERRUPT_PRIORITY) << 24UL)

static int LP_RTC_WaitForSynchro()
{
	uint32_t tickstart = 0U;

	/* Clear RSF flag */
	CLEAR_BIT(RTC->CRL, RTC_FLAG_RSF);

	tickstart = HAL_GetTick();

	/* Wait the registers to be synchronised */
	while ((RTC->CRL & RTC_FLAG_RSF) == 0)
	{
		if ((HAL_GetTick() - tickstart) > RTC_TIMEOUT_VALUE)
		{
			return 0;
		}
	}

	return 1;
}

static int LP_RTC_EnterInitMode()
{
	uint32_t tickstart = 0U;

	tickstart = HAL_GetTick();
	/* Wait till RTC is in INIT state and if Time out is reached exit */
	while ((RTC->CRL & RTC_CRL_RTOFF) == 0)
	{
		if ((HAL_GetTick() - tickstart) > RTC_TIMEOUT_VALUE)
		{
			return 0;
		}
	}

	/* Disable the write protection for RTC registers */
	SET_BIT(RTC->CRL, RTC_CRL_CNF);

	return 1;
}

static int LP_RTC_ExitInitMode()
{
	uint32_t tickstart = 0U;

	/* Enable the write protection for RTC registers */
	CLEAR_BIT(RTC->CRL, RTC_CRL_CNF);

	tickstart = HAL_GetTick();
	/* Wait till RTC is in INIT state and if Time out is reached exit */
	while ((RTC->CRL & RTC_CRL_RTOFF) == 0)
	{
		if ((HAL_GetTick() - tickstart) > RTC_TIMEOUT_VALUE)
		{
			return 0;
		}
	}

	return 1;
}

static void LP_RTC_WriteAlarm(uint32_t AlarmCounter)
{
	/* Set Initialization mode */
	if (LP_RTC_EnterInitMode())
	{
		/* Set RTC COUNTER MSB word */
		WRITE_REG(RTC->ALRH, (AlarmCounter >> 16U));
		/* Set RTC COUNTER LSB word */
		WRITE_REG(RTC->ALRL, (AlarmCounter & RTC_ALRL_RTC_ALR));

		/* Wait for synchro */
		LP_RTC_ExitInitMode();
	}
}

static uint32_t LP_RTC_ReadCounter()
{
	uint16_t high1 = 0U, high2 = 0U, low = 0U;
	uint32_t timecounter = 0U;

	high1 = RTC->CNTH;
	low = RTC->CNTL;
	high2 = RTC->CNTH;

	if (high1 != high2)
	{
		/* In this case the counter roll over during reading of CNTL and CNTH registers,
       read again CNTL register then return the counter value */
		timecounter = (((uint32_t)high2 << 16U) | RTC->CNTL);
	}
	else
	{
		/* No counter roll over during reading of CNTL and CNTH registers, counter
       value is equal to first value of CNTL and CNTH */
		timecounter = (((uint32_t)high1 << 16U) | low);
	}

	return timecounter;
}

static uint32_t LP_RTC_ReadAlarm()
{
	uint16_t high1 = 0U, low = 0U;

	high1 = READ_REG(RTC->ALRH & RTC_CNTH_RTC_CNT);
	low = READ_REG(RTC->ALRL & RTC_CNTL_RTC_CNT);

	return (((uint32_t)high1 << 16U) | low);
}

static void LP_RTC_WriteCounter(uint32_t CounterValue)
{
	/* Set Initialization mode */
	if (LP_RTC_EnterInitMode())
	{
		/* Set RTC COUNTER MSB word */
		RTC->CNTH = CounterValue >> 16U;
		/* Set RTC COUNTER LSB word */
		RTC->CNTL = CounterValue & RTC_CNTL_RTC_CNT;

		/* Wait for synchro */
		LP_RTC_ExitInitMode();
	}
}

void LP_Init()
{
	__HAL_RCC_AFIO_CLK_ENABLE();	//Clock enable
	RTC->CRH = RTC_CRH_ALRIE;		//Alarm interrupt enable
	NVIC_EnableIRQ(RTC_Alarm_IRQn); //
	SET_BIT(EXTI->IMR, EXTI_IMR_IM17);	//EXTI channel 17 enable
	SET_BIT(EXTI->RTSR, EXTI_RTSR_RT17);//
	//LP_RTC_WaitForSynchro();
	//LP_RTC_WriteCounter(0);
	//LP_RTC_WriteAlarm(LP_RTC_ReadCounter() + 10);
}

void RTC_IRQHandler()
{
	if (RTC->CRL & RTC_CRL_SECF)
	{
		//second isr
		RTC->CRL &= ~RTC_CRL_SECF;
		//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_4);
		//HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}
}

void RTC_Alarm_IRQHandler()
{
	EXTI->PR = EXTI_PR_PIF17;
	RTC->CRL &= ~RTC_CRL_ALRF;

	portYIELD();
	//LP_RTC_WriteAlarm(LP_RTC_ReadCounter() + 10);
}

void LP_StopMode()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	/* Clear PDDS bit in PWR register to specify entering in STOP mode when CPU enter in Deepsleep */
	CLEAR_BIT(PWR->CR, PWR_CR_PDDS);

	/* Select the voltage regulator mode by setting LPDS bit in PWR register according to Regulator parameter value */
	MODIFY_REG(PWR->CR, PWR_CR_LPDS, PWR_LOWPOWERREGULATOR_ON);

	/* Set SLEEPDEEP bit of Cortex System Control Register */
	SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));

	/* Request Wait For Interrupt */
	__DSB();
	__WFI();
	__ISB();
	/* Reset SLEEPDEEP bit of Cortex System Control Register */
	CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
	uint32_t ulReloadValue, ulCompleteTickPeriods, ulCompletedSysTickDecrements;
	TickType_t xModifiableIdleTime;

	/* Make sure the SysTick reload value does not overflow the counter. */
	if (xExpectedIdleTime > xMaximumPossibleSuppressedTicks)
	{
		xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
	}

	/* Stop the SysTick momentarily.  The time the SysTick is stopped for
		is accounted for as best it can be, but using the tickless mode will
		inevitably result in some tiny drift of the time maintained by the
		kernel with respect to calendar time. */
	portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;

	/* Calculate the reload value required to wait xExpectedIdleTime
		tick periods.  -1 is used because this code will execute part way
		through one of the tick periods. */
	ulReloadValue = LP_RTC_ReadCounter() + portNVIC_SYSTICK_CURRENT_VALUE_REG * RTC_FREQENCY / SYSTICK_PERIOD / 1000 + ((xExpectedIdleTime - 1UL) * RTC_FREQENCY / 1000);
	if (ulReloadValue > ulStoppedTimerCompensation)
	{
		ulReloadValue -= ulStoppedTimerCompensation;
	}

	/* Enter a critical section but don't use the taskENTER_CRITICAL()
		method as that will mask interrupts that should exit sleep mode. */
	__asm volatile("cpsid i" ::
					   : "memory");
	__asm volatile("dsb");
	__asm volatile("isb");

	/* If a context switch is pending or a task is waiting for the scheduler
		to be unsuspended then abandon the low power entry. */
	if (eTaskConfirmSleepModeStatus() == eAbortSleep)
	{
		/* Restart from whatever is left in the count register to complete
			this tick period. */

		portNVIC_SYSTICK_LOAD_REG = portNVIC_SYSTICK_CURRENT_VALUE_REG;
		portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

		/* Restart SysTick. */
		portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

		/* Reset the reload register to the value required for normal tick
			periods. */
		portNVIC_SYSTICK_LOAD_REG = SYSTICK_PERIOD - 1UL;

		/* Re-enable interrupts - see comments above the cpsid instruction()
			above. */
		__asm volatile("cpsie i" ::
						   : "memory");
	}
	else
	{
		/* Set the new reload value. */
		//portNVIC_SYSTICK_LOAD_REG = ulReloadValue;
		LP_RTC_WriteAlarm(ulReloadValue);

		/* Clear the SysTick count flag and set the count value back to
			zero. */
		//portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

		/* Restart SysTick. */
		//portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

		/* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
			set its parameter to 0 to indicate that its implementation contains
			its own wait for interrupt or wait for event instruction, and so wfi
			should not be executed again.  However, the original expected idle
			time variable must remain unmodified, so a copy is taken. */
		xModifiableIdleTime = xExpectedIdleTime;
		configPRE_SLEEP_PROCESSING(&xModifiableIdleTime);
		if (xModifiableIdleTime > 0)
		{
			/*__asm volatile("dsb" ::: "memory");
			__asm volatile("wfi");
			__asm volatile("isb");*/
			//			HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);	//						*******************
			LP_StopMode();
		}
		configPOST_SLEEP_PROCESSING(&xExpectedIdleTime);

		/* Re-enable interrupts to allow the interrupt that brought the MCU
			out of sleep mode to execute immediately.  see comments above
			__disable_interrupt() call above. */
		//portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
		//portNVIC_SYSTICK_LOAD_REG = SYSTICK_PERIOD;
		//portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;
		__asm volatile("cpsie i" ::
						   : "memory");
		__asm volatile("dsb");
		__asm volatile("isb");

		/* Disable interrupts again because the clock is about to be stopped
			and interrupts that execute while the clock is stopped will increase
			any slippage between the time maintained by the RTOS and calendar
			time. */
		__asm volatile("cpsid i" ::
						   : "memory");
		__asm volatile("dsb");
		__asm volatile("isb");

		/* Disable the SysTick clock without reading the
			portNVIC_SYSTICK_CTRL_REG register to ensure the
			portNVIC_SYSTICK_COUNT_FLAG_BIT is not cleared if it is set.  Again,
			the time the SysTick is stopped for is accounted for as best it can
			be, but using the tickless mode will inevitably result in some tiny
			drift of the time maintained by the kernel with respect to calendar
			time*/

		//portNVIC_SYSTICK_CTRL_REG = (portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT);
		//portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;

		/* Determine if the SysTick clock has already counted to zero and
			been set back to the current reload value (the reload back being
			correct for the entire expected idle time) or if the SysTick is yet
			to count to zero (in which case an interrupt other than the SysTick
			must have brought the system out of sleep mode). */
		LP_RTC_WaitForSynchro();
		int32_t time_delta = LP_RTC_ReadAlarm() - LP_RTC_ReadCounter();
		//if ((portNVIC_SYSTICK_CTRL_REG & portNVIC_SYSTICK_COUNT_FLAG_BIT) != 0)
		if (time_delta < 0)
		{
			int32_t lCalculatedLoadValue;

			/* The tick interrupt is already pending, and the SysTick count
				reloaded with ulReloadValue.  Reset the
				portNVIC_SYSTICK_LOAD_REG with whatever remains of this tick
				period. */
			//ulCalculatedLoadValue = (ulTimerCountsForOneTick - 1UL) - (ulReloadValue - portNVIC_SYSTICK_CURRENT_VALUE_REG);
			lCalculatedLoadValue = SYSTICK_PERIOD + (time_delta * SYSTICK_PERIOD * 1000 / RTC_FREQENCY);
			if (lCalculatedLoadValue < 0)
				lCalculatedLoadValue = SYSTICK_PERIOD - 1;
			/* Don't allow a tiny value, or values that have somehow
				underflowed because the post sleep hook did something
				that took too long. */
			//if ((ulCalculatedLoadValue < ulStoppedTimerCompensation) || (ulCalculatedLoadValue > ulTimerCountsForOneTick))
			//{
			//	ulCalculatedLoadValue = (ulTimerCountsForOneTick - 1UL);
			//}

			portNVIC_SYSTICK_LOAD_REG = lCalculatedLoadValue;

			/* As the pending tick will be processed as soon as this
				function exits, the tick value maintained by the tick is stepped
				forward by one less than the time spent waiting. */
			ulCompleteTickPeriods = xExpectedIdleTime;
		}
		else
		{
			/* Something other than the tick interrupt ended the sleep.
				Work out how long the sleep lasted rounded to complete tick
				periods (not the ulReload value which accounted for part
				ticks). */
			ulCompletedSysTickDecrements = (xExpectedIdleTime * RTC_FREQENCY / 1000) - time_delta;

			/* How many complete tick periods passed while the processor
				was waiting? */
			ulCompleteTickPeriods = ulCompletedSysTickDecrements * 1000 / RTC_FREQENCY;
			uint32_t SYSTICK_Correction = (((ulCompleteTickPeriods + 1) * RTC_FREQENCY / 1000) - ulCompletedSysTickDecrements) * SYSTICK_PERIOD * 1000 / RTC_FREQENCY;

			/* The reload value is set to whatever fraction of a single tick
				period remains. */
			//portNVIC_SYSTICK_LOAD_REG = ((ulCompleteTickPeriods + 1UL) * ulTimerCountsForOneTick) - ulCompletedSysTickDecrements;
			portNVIC_SYSTICK_LOAD_REG = SYSTICK_Correction;
		}

		/* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
			again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
			value. */
		portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
		portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;
		portNVIC_SYSTICK_LOAD_REG = SYSTICK_PERIOD - 1UL;
		if (ulCompleteTickPeriods > 1)
			vTaskStepTick(ulCompleteTickPeriods - 1);
		if (ulCompleteTickPeriods)
			if (xTaskIncrementTick() != pdFALSE)
			{
				/* A context switch is required.  Context switching is performed in
			the PendSV interrupt.  Pend the PendSV interrupt. */
				portYIELD();
			}

		/* Exit with interrpts enabled. */
		__asm volatile("cpsie i" ::
						   : "memory");
	}
}

uint64_t LP_GetCurrentTime()
{
	HAL_PWR_EnableBkUpAccess();
	LP_RTC_WaitForSynchro();
	const uint32_t last_rtc = ((uint32_t)(BKP->DR10) << 16) | (BKP->DR9);
	const uint32_t current_rtc = LP_RTC_ReadCounter();
	const uint32_t delta_rtc = current_rtc - last_rtc;
	BKP->DR10 = (current_rtc >> 16) & 0xFFFF;
	BKP->DR9 = current_rtc & 0xFFFF;
	const uint64_t last_time = ((uint64_t)(BKP->DR8) << 48) | ((uint64_t)(BKP->DR7) << 32) | ((uint64_t)(BKP->DR6) << 16) | BKP->DR5;
	const uint64_t current_time = last_time + delta_rtc;
	BKP->DR8 = (current_time >> 48) & 0xFFFF;
	BKP->DR7 = (current_time >> 32) & 0xFFFF;
	BKP->DR6 = (current_time >> 16) & 0xFFFF;
	BKP->DR5 = current_time & 0xFFFF;
	return current_time * 1000 / RTC_FREQENCY;
}

void LP_SetCurrentTime(uint64_t new_time)
{
	HAL_PWR_EnableBkUpAccess();
	LP_RTC_WaitForSynchro();
	const uint32_t current_rtc = LP_RTC_ReadCounter();
	BKP->DR10 = (current_rtc >> 16) & 0xFFFF;
	BKP->DR9 = current_rtc & 0xFFFF;
	const uint64_t current_time = new_time * RTC_FREQENCY / 1000;
	BKP->DR8 = (current_time >> 48) & 0xFFFF;
	BKP->DR7 = (current_time >> 32) & 0xFFFF;
	BKP->DR6 = (current_time >> 16) & 0xFFFF;
	BKP->DR5 = current_time & 0xFFFF;
}
