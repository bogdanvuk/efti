
#include "xttcps.h"
#include "xil_printf.h"
#include "util.h"

#define TTC_DEVICE_ID	    XPAR_XTTCPS_0_DEVICE_ID
#define TTC_INTR_ID		    XPAR_XTTCPS_0_INTR

XTtcPs_Config *Config;
XTtcPs Timer;

int timing_init()
{
	//initialise the timer
	Config = XTtcPs_LookupConfig(TTC_DEVICE_ID);
	XTtcPs_CfgInitialize(&Timer, Config, Config->BaseAddress);
	Timer.IsReady = XIL_COMPONENT_IS_READY;

	XTtcPs_Stop(&Timer);
	XTtcPs_ResetCounterValue(&Timer);

	XTtcPs_SelfTest(&Timer);

	XTtcPs_SetOptions(&Timer, XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE);

	XTtcPs_SetInterval(&Timer, 65535);
	XTtcPs_SetPrescaler(&Timer, 15);

	XTtcPs_Start(&Timer);
	XTtcPs_ResetCounterValue(&Timer);

	return 0;
}

int timing_close()
{
	XTtcPs_Stop(&Timer);
	return 0;
}

void timing_print()
{
	uint32_t ticks = XTtcPs_GetCounterValue(&Timer);

	float t = (ticks / (111111111.0 / 65536.0));

	xil_printf("Time: ");
	print_float(t, 1000);
	xil_printf("\n\r");
}
