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
#include "tree.h"
#include <string.h>

#if (EFTI_PC == 1)
void efti_printf( const char *format, ...)
{
	va_list args;
	va_start (args, format);
	vprintf (format, args);
	va_end (args);
}

char depth[ 2056 ];
int di;

void Push( char c )
{
    depth[ di++ ] = ' ';
    depth[ di++ ] = c;
    depth[ di++ ] = ' ';
    depth[ di++ ] = ' ';
    depth[ di ] = 0;
}

void Pop( )
{
    depth[ di -= 4 ] = 0;
}

int print_t( tree_node* tree, int attr_cnt)
{
    char p[16];
    char w[12];
    char node_desc[2048];

    sprintf(p, "%p", tree);
    /* sprintf(node_desc, "(%s)", &p[5]); */
    sprintf(node_desc, "(%d) (%s)", tree->id, &p[5]);

    if ((attr_cnt > 0) && (tree->left != NULL)) {
        strcat(node_desc, " [");
        /* for (int i = 0; i < attr_cnt; i++) { */
        /*     /\* sprintf(w, "%d ", tree->weights[i]); *\/ */
        /*     sprintf(w, "%f ", tree->weights[i]); */
        /*     strcat(node_desc, w); */
        /* } */

        strcat(node_desc, "], ");
        /* sprintf(w, "%d", tree->weights[NUM_ATTRIBUTES]); */
        sprintf(w, "%f", tree->weights[NUM_ATTRIBUTES]);
        strcat(node_desc, w);
    }

    efti_printf(node_desc);
    efti_printf("\n");

    if ( tree->left )
    {
        efti_printf( "%s `--", depth );
        Push( '|' );
        print_t( tree->left, attr_cnt );
        Pop( );

        efti_printf( "%s `--", depth );
        Push( ' ' );
        print_t( tree->right, attr_cnt );
        Pop( );
    }
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
