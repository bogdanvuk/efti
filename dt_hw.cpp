/*
 * dt_hw.cpp
 *
 *  Created on: Feb 2, 2015
 *      Author: bvukobratovic
 */

#include "dt_hw.h"
#include "xil_mmu.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xaxidma.h"

/*
 * Device hardware build related constants.
 */

#define DMA_DEV_ID		0

#define MEM_BASE_ADDR		0x11000000

#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00000000)
#define RX_BD_SPACE_HIGH	(MEM_BASE_ADDR + 0x00001FFF)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

XAxiDma AxiDma;

#define DT_HW_AXI_BASE_ADDR				0x40000000

#define DT_HW_AXI_CONF_STAT_OFFSET		0x00000000
#define DT_HW_AXI_CONF_STAT_BASE_ADDR	(DT_HW_AXI_BASE_ADDR + DT_HW_AXI_CONF_STAT_OFFSET)

#define DT_HW_CONF1_OFFSET				0x00000000
#define DT_HW_CONF1_ADDR				(DT_HW_CONF1_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)
#define DT_HW_CONF1_RUN_BIT				0
#define DT_HW_CONF1_RST_BIT				1

#define DT_HW_INST_NUM_OFFSET			0x00000004
#define DT_HW_INST_NUM_ADDR				(DT_HW_INST_NUM_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)

#define DT_HW_AXI_PROC_CNT_LOW_OFFSET	0x00000010
#define DT_HW_AXI_PROC_CNT_LOW_ADDR		(DT_HW_AXI_PROC_CNT_LOW_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)

#define DT_HW_AXI_PROC_CNT_HIGH_OFFSET	0x00000014
#define DT_HW_AXI_PROC_CNT_HIGH_ADDR	(DT_HW_AXI_PROC_CNT_HIGH_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)

#define DT_HW_STAT1_OFFSET				0x00000018
#define DT_HW_STAT1_ADDR				(DT_HW_STAT1_OFFSET + DT_HW_AXI_CONF_STAT_BASE_ADDR)
#define DT_HW_STAT1_FINISHED_BIT		0

#include "xil_exception.h"

T_Dt_Hw_State dt_hw_state;

T_Dt_Hw_Config dt_hw_config;

#define MAX_PKT_LEN		0x1000

int Status;
u8 * volatile RxBufferPtr;
u8 Value;

static int RxSetup(XAxiDma * AxiDmaInstPtr);

//uint_fast16_t node_categories_distrib[256][8];
uint_fast16_t** node_categories_distrib;

int log2(unsigned value) {
	int msb_index = -1;
	while(value) {
			value = value >> 1;
			++msb_index;
	}
	return msb_index;
}

int dt_hw_init(T_Dt_Hw_Config* cfg)
{
	Xil_SetTlbAttributes(DT_HW_AXI_BASE_ADDR, 0xC06);
//	Xil_DCacheDisable();

	dt_hw_config = *cfg;

	dt_hw_proc_cnt_reset();

	Xil_Out32(DT_HW_INST_NUM_ADDR, dt_hw_config.inst_num);

	XAxiDma_Config *CfgPtr;

	/* Initialize the XAxiDma device.
		 */
	CfgPtr = XAxiDma_LookupConfig(DMA_DEV_ID);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DMA_DEV_ID);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	XAxiDma_Reset(&AxiDma);

	if(XAxiDma_HasSg(&AxiDma)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);
	
	RxBufferPtr = (u8 *)RX_BUFFER_BASE;

//	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
//				dt_hw_config.inst_num*4, XAXIDMA_DEVICE_TO_DMA);

	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
						0x1000, XAXIDMA_DEVICE_TO_DMA);

	volatile uint32_t* idle_steps_strg = (u32*) 0x1f000000;
	*idle_steps_strg = 0;

	node_categories_distrib   = new uint_fast16_t*[256];

    for (int i = 0; i < 256; ++i)
    {
        node_categories_distrib[i] = new uint_fast16_t[32];
    }

	return 0;
}

static uint32_t calc_latency(void)
{
	unsigned node_latency = log2(dt_hw_config.attributes_num) + 2;
	return dt_hw_config.tree_depth_max * node_latency;
}

void dt_hw_conf1_set(void)
{
	uint32_t conf1;

	conf1 = (calc_latency() << 16);

	switch(dt_hw_state.state)
	{
		case DT_HW_RESET: conf1 |= 0x00000002; break;
		case DT_HW_IDLE: break;
		case DT_HW_RUNNING : conf1 |= 0x00000001; break;
		case DT_HW_FINISHED : break;
	}

	Xil_Out32(DT_HW_CONF1_ADDR, conf1);
}


void dt_hw_proc_cnt_run(void)
{
	dt_hw_state.state = DT_HW_RUNNING;
	dt_hw_conf1_set();
}

void dt_hw_proc_cnt_reset(void)
{
	dt_hw_state.state = DT_HW_RESET;
	dt_hw_conf1_set();
}

uint64_t dt_hw_proc_cnt_get(void)
{
	return ((uint64_t) Xil_In32(DT_HW_AXI_PROC_CNT_HIGH_ADDR) << 32) + Xil_In32(DT_HW_AXI_PROC_CNT_LOW_ADDR);
}

//uint32_t dt_hw_eval(uint_fast16_t node_categories_distrib[][64], uint_fast16_t categories[])
uint32_t dt_hw_eval(uint_fast16_t leaves_array[], uint_fast16_t leaves_cnt, uint_fast16_t categories[])
{
	volatile uint32_t* idle_steps_strg = (u32*) 0x1f000000;
	uint32_t idle_steps = 0;
	volatile uint32_t* rxBuf = (u32*) RxBufferPtr;

	for (unsigned i = 0; i < leaves_cnt; i++)
	{
		uint32_t node = leaves_array[i];
		memset(node_categories_distrib[node], 0, sizeof(uint_fast16_t)*8);
	}

//	XAxiDma_Reset(&AxiDma);

//	*(u32* volatile) 0x40400030 = 0x10003;
//	*(u32* volatile) 0x40400054 = 0x11300000;
//	*(u32* volatile) 0x40400058 = dt_hw_config.inst_num*4;

//	Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
//					(dt_hw_config.inst_num - 1)*4, XAXIDMA_DEVICE_TO_DMA);

//	for (uint32_t i = 0 ; i < dt_hw_config.inst_num; i++)
//	{
//		(*rxBuf++) = 0;
//	}
//
//	Xil_DCacheInvalidate();

//	Xil_DCacheInvalidateRange((u32)RxBufferPtr, dt_hw_config.inst_num*4);

	//*(u32* volatile) 0x40400058 = 0x1000;

	dt_hw_proc_cnt_reset();
	dt_hw_proc_cnt_run();

//	Xil_DCacheInvalidateRange((u32)RxBufferPtr, 256);

	while (dt_hw_state.state == DT_HW_RUNNING)
	{
		if (Xil_In32(DT_HW_STAT1_ADDR) & (1 << DT_HW_STAT1_FINISHED_BIT))
		{
			dt_hw_state.state = DT_HW_FINISHED;
		}
	}

//	idle_steps = *(volatile u32*) 0xf8001018;

//	rxBuf = (u32*) RxBufferPtr;
//	for (uint32_t i = 0 ; i < dt_hw_config.inst_num - 10; i++)
//	{
//		uint32_t node = (*rxBuf++);
//		uint32_t categ = categories[i];
//		node_categories_distrib[node][categ]++;
////		node_categories_distrib[0][node]++;
//	}
//
	uint32_t hits = 0;
//	for (unsigned i = 0; i < leaves_cnt; i++)
//	{
//		uint32_t node = leaves_array[i];
//		uint_fast16_t* node_distrib = node_categories_distrib[node];
////		uint_fast16_t dominant_category_id = 0;
//		uint_fast16_t dominant_category_cnt = *node_distrib;
//
//		for (unsigned j = 1; j < 4; j++)
//		{
//			uint_fast16_t categ_cnt = *(node_distrib++);
//			if (dominant_category_cnt < categ_cnt)
//			{
////				dominant_category_id = j;
//				dominant_category_cnt = categ_cnt;
//			}
//		}
//
//		hits += dominant_category_cnt;
////		this->node_categories[i] = dominant_category_id;
//	}

//	idle_steps -= *(volatile u32*) 0xf8001018;

//	*idle_steps_strg = *idle_steps_strg + idle_steps;

//	while (((* (volatile u32 *) 0x40400034) & 0x3) == 0)
//	{
//
//	}

//	while (XAxiDma_Busy(&AxiDma,XAXIDMA_DEVICE_TO_DMA)){
//			a = XAxiDma_Busy(&AxiDma,XAXIDMA_DEVICE_TO_DMA);
//			a = * (volatile u32 *) 0x40400034;
//			a = XAxiDma_ReadReg((&AxiDma)->RegBase + (XAXIDMA_RX_OFFSET * XAXIDMA_DEVICE_TO_DMA), XAXIDMA_SR_OFFSET);
//			a = XAxiDma_ReadReg((&AxiDma)->RegBase + (XAXIDMA_RX_OFFSET * XAXIDMA_DEVICE_TO_DMA), XAXIDMA_SR_OFFSET) & XAXIDMA_IDLE_MASK;
//			a = (XAxiDma_ReadReg((&AxiDma)->RegBase + (XAXIDMA_RX_OFFSET * XAXIDMA_DEVICE_TO_DMA), XAXIDMA_SR_OFFSET) & XAXIDMA_IDLE_MASK) ? FALSE : TRUE;
//	}

//	Xil_DCacheInvalidateRange((u32)RxBufferPtr, MAX_PKT_LEN);

//	memcpy((unsigned int *)0x40030000, classes, dt_hw_config.inst_num);

	return hits;
}

/*****************************************************************************/
/**
*
* This function sets up RX channel of the DMA engine to be ready for packet
* reception
*
* @param	AxiDmaInstPtr is the pointer to the instance of the DMA engine.
*
* @return	XST_SUCCESS if the setup is successful, XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int RxSetup(XAxiDma * AxiDmaInstPtr)
{
	XAxiDma_BdRing *RxRingPtr;
	int Delay = 0;
	int Coalesce = 1;
	int Status;
	XAxiDma_Bd BdTemplate;
	XAxiDma_Bd *BdPtr;
	XAxiDma_Bd *BdCurPtr;
	u32 BdCount;
	u32 FreeBdCount;
	u32 RxBufferPtr;
	unsigned Index;

	RxRingPtr = XAxiDma_GetRxRing(&AxiDma);

	/* Disable all RX interrupts before RxBD space setup */

	XAxiDma_BdRingIntDisable(RxRingPtr, XAXIDMA_IRQ_ALL_MASK);

	/* Set delay and coalescing */
	XAxiDma_BdRingSetCoalesce(RxRingPtr, Coalesce, Delay);

	/* Setup Rx BD space */
	BdCount = XAxiDma_BdRingCntCalc(XAXIDMA_BD_MINIMUM_ALIGNMENT,
				RX_BD_SPACE_HIGH - RX_BD_SPACE_BASE + 1);

	Status = XAxiDma_BdRingCreate(RxRingPtr, RX_BD_SPACE_BASE,
				RX_BD_SPACE_BASE,
				XAXIDMA_BD_MINIMUM_ALIGNMENT, BdCount);

	if (Status != XST_SUCCESS) {
		xil_printf("RX create BD ring failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/*
	 * Setup an all-zero BD as the template for the Rx channel.
	 */
	XAxiDma_BdClear(&BdTemplate);

	Status = XAxiDma_BdRingClone(RxRingPtr, &BdTemplate);
	if (Status != XST_SUCCESS) {
		xil_printf("RX clone BD failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Attach buffers to RxBD ring so we are ready to receive packets */

	FreeBdCount = XAxiDma_BdRingGetFreeCnt(RxRingPtr);

	Status = XAxiDma_BdRingAlloc(RxRingPtr, FreeBdCount, &BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX alloc BD failed %d\r\n", Status);

		return XST_FAILURE;
	}

	BdCurPtr = BdPtr;
	RxBufferPtr = RX_BUFFER_BASE;
	for (Index = 0; Index < FreeBdCount; Index++) {
		Status = XAxiDma_BdSetBufAddr(BdCurPtr, RxBufferPtr);

		if (Status != XST_SUCCESS) {
			xil_printf("Set buffer addr %x on BD %x failed %d\r\n",
			    (unsigned int)RxBufferPtr,
			    (unsigned int)BdCurPtr, Status);

			return XST_FAILURE;
		}

		Status = XAxiDma_BdSetLength(BdCurPtr, MAX_PKT_LEN,
				RxRingPtr->MaxTransferLen);
		if (Status != XST_SUCCESS) {
			xil_printf("Rx set length %d on BD %x failed %d\r\n",
			    MAX_PKT_LEN, (unsigned int)BdCurPtr, Status);

			return XST_FAILURE;
		}

		/* Receive BDs do not need to set anything for the control
		 * The hardware will set the SOF/EOF bits per stream status
		 */
		XAxiDma_BdSetCtrl(BdCurPtr, 0);
		XAxiDma_BdSetId(BdCurPtr, RxBufferPtr);

		RxBufferPtr += MAX_PKT_LEN;
		BdCurPtr = XAxiDma_BdRingNext(RxRingPtr, BdCurPtr);
	}

	/* Clear the receive buffer, so we can verify data
	 */
	memset((void *)RX_BUFFER_BASE, 0, MAX_PKT_LEN);

	Status = XAxiDma_BdRingToHw(RxRingPtr, FreeBdCount,
						BdPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX submit hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	/* Start RX DMA channel */
	Status = XAxiDma_BdRingStart(RxRingPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("RX start hw failed %d\r\n", Status);

		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* The example to do the simple transfer through polling. The constant
* NUMBER_OF_TRANSFERS defines how many times a simple transfer is repeated.
*
* @param	DeviceId is the Device Id of the XAxiDma instance
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if error occurs
*
* @note		None
*
*
******************************************************************************/

#if 0
int XAxiDma_SimplePollExample(u16 DeviceId)
{
	XAxiDma_Config *CfgPtr;
	int Status;
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;
	u8 *TxBufferPtr;
	u8 *RxBufferPtr;
	u8 Value;

	TxBufferPtr = (u8 *)TX_BUFFER_BASE ;
	RxBufferPtr = (u8 *)RX_BUFFER_BASE;

	/* Initialize the XAxiDma device.
	 */
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if(XAxiDma_HasSg(&AxiDma)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	Value = TEST_START_VALUE;

	for(Index = 0; Index < MAX_PKT_LEN; Index ++) {
			TxBufferPtr[Index] = Value;

			Value = (Value + 1) & 0xFF;
	}
	/* Flush the SrcBuffer before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((u32)TxBufferPtr, MAX_PKT_LEN);

	for(Index = 0; Index < Tries; Index ++) {


		Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) RxBufferPtr,
					MAX_PKT_LEN, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma,(u32) TxBufferPtr,
					MAX_PKT_LEN, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while ((XAxiDma_Busy(&AxiDma,XAXIDMA_DEVICE_TO_DMA)) ||
			(XAxiDma_Busy(&AxiDma,XAXIDMA_DMA_TO_DEVICE))) {
				/* Wait */
		}

		Status = CheckData();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}

	/* Test finishes successfully
	 */
	return XST_SUCCESS;
}
#endif

//	/*
//	 * Unmask Data Abort in CPSR
//	 */
//	my_mtcpsr(mfcpsr() & ~ (XREG_CPSR_DATA_ABORT_ENABLE | XIL_EXCEPTION_FIQ));

//	dt_hw_proc_cnt_reset();
//
//	Xil_Out32(0x40000004, 0x00000064);
//	Xil_Out32(0x40000000, 0x00200001);
//
//	//dt_hw_proc_cnt_run();
//
//	a = Xil_In32(0x40000000);
//	a = Xil_In32(0x40000004);
//
//	a = dt_hw_proc_cnt_get();


