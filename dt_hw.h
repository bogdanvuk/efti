/*
 * dt_hw.h
 *
 *  Created on: Feb 2, 2015
 *      Author: bvukobratovic
 */

#ifndef DT_HW_H_
#define DT_HW_H_

#include <stdint.h>

typedef enum
{
	DT_HW_RESET		= 0,
	DT_HW_IDLE		= 1,
	DT_HW_RUNNING	= 2,
	DT_HW_FINISHED	= 3
} E_Dt_Hw_State;

typedef struct
{
	uint32_t inst_num;
	uint32_t tree_depth_max;
	uint32_t attributes_num;
	uint32_t nodes_num;
} T_Dt_Hw_Config;

typedef struct
{
	uint32_t state;
} T_Dt_Hw_State;

int dt_hw_init(T_Dt_Hw_Config* cfg);

void dt_hw_proc_cnt_run(void);
void dt_hw_proc_cnt_reset(void);

uint64_t dt_hw_proc_cnt_get(void);

uint32_t dt_hw_eval(uint_fast16_t leaves_array[], uint_fast16_t leaves_cnt, uint_fast16_t categories[]);

#endif /* DT_HW_H_ */
