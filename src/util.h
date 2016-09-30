/*
 * util.h
 *
 *  Created on: Feb 12, 2015
 *      Author: bvukobratovic
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdint.h>
#include "tree.h"

void efti_printf(const char *format, ...);
int print_t(tree_node *tree, int attr_cnt);

#endif /* UTIL_H_ */
