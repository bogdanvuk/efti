/*
 * util.c
 *
 *  Created on: Feb 12, 2015
 *      Author: bvukobratovic
 */
#include "util.h"
#include "xil_printf.h"
#include "stdio.h"

char buffer[1024];

void ser_printf( const char *format, ...)
{
	va_list args;
	va_start (args, format);
	vsprintf (buffer,format, args);

	char* cout = buffer;

	while (*cout)
	{
		outbyte(*(cout++));
	}

	va_end (args);
}

void print_float(float num, float res)
{
	uint32_t num_int = (uint32_t) num;
	uint32_t num_dec = (uint32_t) ((num - num_int) * res);

	xil_printf("%d.%d", num_int, num_dec);
}


