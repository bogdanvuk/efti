
#include "xttcps.h"
#include "xil_printf.h"
#include "util.h"

#define TTC_DEVICE_ID	    XPAR_XTTCPS_0_DEVICE_ID
#define TTC_INTR_ID		    XPAR_XTTCPS_0_INTR

uint32_t ttc_device[3] = {
	XPAR_XTTCPS_0_DEVICE_ID,
	XPAR_XTTCPS_1_DEVICE_ID,
	XPAR_XTTCPS_2_DEVICE_ID
};

XTtcPs_Config* ttc_config[3];
XTtcPs ttc[3];

int timing_init(uint32_t id, uint32_t interval, uint32_t prescaler)
{
	//initialise the timer
	ttc_config[id] = XTtcPs_LookupConfig(ttc_device[id]);
	XTtcPs_CfgInitialize(&ttc[id], ttc_config[id], ttc_config[id]->BaseAddress);
	ttc[id].IsReady = XIL_COMPONENT_IS_READY;

	XTtcPs_Stop(&ttc[id]);
	XTtcPs_ResetCounterValue(&ttc[id]);

	XTtcPs_SelfTest(&ttc[id]);

	XTtcPs_SetOptions(&ttc[id], XTTCPS_OPTION_INTERVAL_MODE | XTTCPS_OPTION_WAVE_DISABLE);

	XTtcPs_SetInterval(&ttc[id], interval);
	XTtcPs_SetPrescaler(&ttc[id], prescaler);

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

//void timing_print()
//{
//	uint32_t ticks = XTtcPs_GetCounterValue(&Timer);
//
//	float t = (ticks / (111111111.0 / 65536.0));
//
//	xil_printf("Time: ");
//	print_float(t, 1000);
//	xil_printf("\n\r");
//}
