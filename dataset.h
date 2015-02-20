/*
 * dataset.h
 *
 *  Created on: Feb 19, 2015
 *      Author: bvukobratovic
 */

#ifndef DATASET_H_
#define DATASET_H_

typedef struct
{
	uint32_t inst_cnt;
	uint32_t attr_cnt;
	uint32_t categ_max;

	char	name[32];

	int32_t* instances;
	uint32_t* classes;
} T_Dataset;

#endif /* DATASET_H_ */
