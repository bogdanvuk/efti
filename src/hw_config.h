/*
 * hw_config.h
 *
 *  Created on: Feb 12, 2015
 *      Author: bvukobratovic
 */

#ifndef HW_CONFIG_H_
#define HW_CONFIG_H_

#define ATTRIBUTE_RES    		16
#define COEF_RES         		16
#define CLASS_RES		 		8
#define NUM_ATTRIBUTES   		25088
#define MAX_TREE_DEPTH			8
#define NUM_NODES		 		(1 << (MAX_TREE_DEPTH - 1))

#define NUM_INST_MAX			4096
// #define NUM_INST_MAX			60000

#define CEILING(x,y) (((x) + (y) - 1) / (y))

#define INST_MEM_BANKS_NUM			CEILING(NUM_ATTRIBUTES * ATTRIBUTE_RES, 32)
#define INST_MEM_ADDR_WIDTH 		12			// ceil(log2(NUM_INST_MAX))

#define DT_MEM_COEF_BANKS_NUM		CEILING((NUM_ATTRIBUTES + 1) * COEF_RES, 32)
#define DT_MEM_CLS_PTR_BANKS_NUM	1
#define DT_MEM_BANKS_NUM			(DT_MEM_COEF_BANKS_NUM + DT_MEM_CLS_PTR_BANKS_NUM)

#define DT_MEM_ADDR_WIDTH			4			// ceil(log2(NUM_NODES))
#define DT_MEM_BANKS_ADDR_WIDTH		5			// ceil(log2(DT_MEM_BANKS_NUM))
#define DT_MEM_TOT_ADDR_WIDTH		(DT_MEM_ADDR_WIDTH + DT_MEM_BANKS_ADDR_WIDTH)

#define DT_CHILD_PTR_DATA_WIDTH		DT_MEM_ADDR_WIDTH

#define DT_ADDER_TREE_DEPTH			5

#endif /* HW_CONFIG_H_ */
