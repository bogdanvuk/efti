/*
 * util.c
 *
 *  Created on: Feb 12, 2015
 *      Author: bvukobratovic
 */
#include "efti_conf.h"
#include "util.h"
#include "stdio.h"
#include "stdarg.h"

#if (EFTI_PC == 1)

void efti_printf( const char *format, ...)
{
	va_list args;
	va_start (args, format);
	vprintf (format, args);
	va_end (args);
}


#else

char buffer[1024];
#include "xil_printf.h"

void efti_printf( const char *format, ...)
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

#endif



