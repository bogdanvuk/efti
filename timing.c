#include "efti_conf.h"
#include <stdint.h>

#if EFTI_PC == 0

#include "xscutimer.h"
#include "xscugic.h"
#include "xttcps.h"
#include "xil_printf.h"

#define STDOUT_IS_PS7_UART
#define UART_DEVICE_ID 0

#define configCPU_CLOCK_HZ						XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ
#define configTICK_RATE_HZ 	10000

void vAssertCalled( const char * pcFile, unsigned long ulLine );
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ );

#define portPRIORITY_SHIFT		3

#define configINTERRUPT_CONTROLLER_BASE_ADDRESS 		( XPAR_PS7_SCUGIC_0_DIST_BASEADDR )
#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET ( -0xf00 )
#define configUNIQUE_INTERRUPT_PRIORITIES				32

#define portLOWEST_INTERRUPT_PRIORITY ( ( ( uint32_t ) configUNIQUE_INTERRUPT_PRIORITIES ) - 1UL )
#define portLOWEST_USABLE_INTERRUPT_PRIORITY ( portLOWEST_INTERRUPT_PRIORITY - 1UL )

#define TTC_DEVICE_ID	    XPAR_XTTCPS_0_DEVICE_ID
#define TTC_INTR_ID		    XPAR_XTTCPS_0_INTR

#define portCPU_IRQ_ENABLE()										\
	__asm volatile ( "CPSIE i" );									\
	__asm volatile ( "DSB" );										\
	__asm volatile ( "ISB" );

uint32_t ttc_device[3] = {
	XPAR_XTTCPS_0_DEVICE_ID,
	XPAR_XTTCPS_1_DEVICE_ID,
	XPAR_XTTCPS_2_DEVICE_ID
};

XTtcPs_Config* ttc_config[3];
XTtcPs ttc[3];
uint32_t ttc_prescaler[3];

uint32_t timer_id = 2;
uint32_t ticks = 0;

#define XSCUTIMER_CLOCK_HZ ( XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2UL )

static XScuTimer xTimer;

void vAssertCalled( const char * pcFile, unsigned long ulLine )
{
	volatile unsigned long ul = 0;

	( void ) pcFile;
	( void ) ulLine;
}

void vClearTickInterrupt( void )
{
	XScuTimer_ClearInterruptStatus( &xTimer );
}

void tick_handler(void)
{
	ticks++;
	vClearTickInterrupt();
}


/*
 * The application must provide a function that configures a peripheral to
 * create the FreeRTOS tick interrupt, then define configSETUP_TICK_INTERRUPT()
 * in FreeRTOSConfig.h to call the function.  This file contains a function
 * that is suitable for use on the Zynq SoC.
 */
void vConfigureTickInterrupt( void )
{
static XScuGic xInterruptController; 	/* Interrupt controller instance */
long xStatus;
extern void FreeRTOS_Tick_Handler( void );
XScuTimer_Config *pxTimerConfig;
XScuGic_Config *pxGICConfig;
const uint8_t ucRisingEdge = 3;

	/* This function is called with the IRQ interrupt disabled, and the IRQ
	interrupt should be left disabled.  It is enabled automatically when the
	scheduler is started. */

	/* Ensure XScuGic_CfgInitialize() has been called.  In this demo it has
	already been called from prvSetupHardware() in main(). */
	pxGICConfig = XScuGic_LookupConfig( XPAR_SCUGIC_SINGLE_DEVICE_ID );
	xStatus = XScuGic_CfgInitialize( &xInterruptController, pxGICConfig, pxGICConfig->CpuBaseAddress );
	configASSERT( xStatus == XST_SUCCESS );
	( void ) xStatus; /* Remove compiler warning if configASSERT() is not defined. */
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
							(Xil_ExceptionHandler)XScuGic_InterruptHandler,
							&xInterruptController);

	/* The priority must be the lowest possible. */
//	XScuGic_SetPriorityTriggerType( &xInterruptController, XPAR_SCUTIMER_INTR, portLOWEST_USABLE_INTERRUPT_PRIORITY << portPRIORITY_SHIFT, ucRisingEdge );

	/* Initialise the timer. */
	pxTimerConfig = XScuTimer_LookupConfig( XPAR_SCUTIMER_DEVICE_ID );
	xStatus = XScuTimer_CfgInitialize( &xTimer, pxTimerConfig, pxTimerConfig->BaseAddr );
	configASSERT( xStatus == XST_SUCCESS );
	( void ) xStatus; /* Remove compiler warning if configASSERT() is not defined. */

	/* Install the FreeRTOS tick handler. */
	xStatus = XScuGic_Connect( &xInterruptController, XPAR_SCUTIMER_INTR, (Xil_ExceptionHandler) tick_handler, ( void * ) &xTimer );
	configASSERT( xStatus == XST_SUCCESS );
	( void ) xStatus; /* Remove compiler warning if configASSERT() is not defined. */

	/* Enable the interrupt for the xTimer in the interrupt controller. */
	XScuGic_Enable( &xInterruptController, XPAR_SCUTIMER_INTR );
	XScuTimer_EnableInterrupt( &xTimer );

	Xil_ExceptionEnable();

	/* Load the timer counter register. */
	XScuTimer_LoadTimer( &xTimer, XSCUTIMER_CLOCK_HZ / configTICK_RATE_HZ );

	/* Enable Auto reload mode. */
	XScuTimer_EnableAutoReload( &xTimer );

	/* Start the timer counter and then wait for it to timeout a number of
	times. */
	XScuTimer_Start( &xTimer );

	/* Enable the interrupt in the xTimer itself. */
//	vClearTickInterrupt();
//	XScuTimer_EnableInterrupt( &xTimer );
//	portCPU_IRQ_ENABLE();
}

int timing_init(uint32_t id, uint32_t interval, uint32_t prescaler)
{
	//initialise the timer
//	ttc_config[id] = XTtcPs_LookupConfig(ttc_device[id]);
//	XTtcPs_CfgInitialize(&ttc[id], ttc_config[id], ttc_config[id]->BaseAddress);
//	ttc[id].IsReady = XIL_COMPONENT_IS_READY;
//
//	XTtcPs_Stop(&ttc[id]);
//	XTtcPs_ResetCounterValue(&ttc[id]);
//
//	XTtcPs_SelfTest(&ttc[id]);
//
//	XTtcPs_SetOptions(&ttc[id], XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE);
//
//	XTtcPs_SetInterval(&ttc[id], interval);
//	XTtcPs_SetPrescaler(&ttc[id], prescaler);
//
//	ttc_prescaler[id] = prescaler;

	vConfigureTickInterrupt();

	return 0;
}

void timing_reset(uint32_t id)
{
	XTtcPs_ResetCounterValue(&ttc[id]);
}

void timing_start(uint32_t id)
{
	XTtcPs_Start(&ttc[id]);
}

void timing_stop(uint32_t id)
{
	XTtcPs_Stop(&ttc[id]);
}

void timing_close(uint32_t id)
{
	XTtcPs_Stop(&ttc[id]);
}

uint32_t timing_count_get(uint32_t id)
{
	return XTtcPs_GetCounterValue(&ttc[id]);
}

float timing_tick2sec(uint32_t ticks)
{
	return ((float) ticks) / configTICK_RATE_HZ;
//	return ticks / (111111111.0 / (1 << (ttc_prescaler[timer_id] + 1)));
}

uint32_t timing_get()
{
	return ticks;
//	return ticks + timing_count_get(timer_id);
}

#elif EFTI_DSP == 1

#include "csl_timer.h"

#define CLK_FREQ 	225e+6
#define CLOCKS_PER_SEC (CLK_FREQ / 4) // / PRESCALER / 2)

TIMER_Handle ttc_handle[3];
uint32_t ttc[3];
uint32_t ttc_prescaler[3];

int timing_init(uint32_t id, uint32_t interval, uint32_t prescaler)
{
	ttc_handle[id] = TIMER_open(id, TIMER_OPEN_RESET);
	TIMER_configArgs(ttc_handle[id], 0x000002c0, 0x10000000, 0x00000000);

	return 0;
}

void timing_reset(uint32_t id)
{
	TIMER_reset(ttc_handle[id]);
	TIMER_configArgs(ttc_handle[id], 0x000002c0, 0x20000000, 0x00000000);
}

void timing_start(uint32_t id)
{
	TIMER_start(ttc_handle[id]);
}

void timing_stop(uint32_t id)
{

}

void timing_close(uint32_t id)
{

}

uint32_t timing_count_get(uint32_t id)
{
	return TIMER_getCount(ttc_handle[id]);
}

float timing_tick2sec(uint32_t id, uint32_t ticks)
{
	return (float) ticks / CLOCKS_PER_SEC;
}

#else

#include <time.h>

clock_t ttc[3];
uint32_t ttc_prescaler[3];

int timing_init(uint32_t id, uint32_t interval, uint32_t prescaler)
{
	ttc[id] = clock();
	ttc_prescaler[id] = prescaler;

	return 0;
}

void timing_reset(uint32_t id)
{
	ttc[id] = clock();
}

void timing_start(uint32_t id)
{
	ttc[id] = clock();
}

void timing_stop(uint32_t id)
{

}

void timing_close(uint32_t id)
{

}

uint32_t timing_get(uint32_t id)
{
	return 0;
//	return clock() - ttc[id];
}

float timing_tick2sec(uint32_t id, uint32_t ticks)
{
	return (float) ticks / CLOCKS_PER_SEC;
}


#endif
