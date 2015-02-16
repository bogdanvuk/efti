/*
 * util.c
 *
 *  Created on: Feb 12, 2015
 *      Author: bvukobratovic
 */
#include "util.h"
#include "xil_printf.h"

void print_float(float num, unsigned res)
{
	uint32_t num_int = (uint32_t) num;
	uint32_t num_dec = (uint32_t) ((num - num_int) * res);

	xil_printf("%d.%d", num_int, num_dec);
}


