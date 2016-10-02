/*
 * timing.h
 *
 *  Created on: Feb 9, 2015
 *      Author: bvukobratovic
 */

#ifndef TIMING_H_
#define TIMING_H_

#include <stdint.h>

int timing_init(uint32_t id, uint32_t interval, uint32_t prescaler);
//void timing_print(uint32_t id);
void timing_reset(uint32_t id);
void timing_start(uint32_t id);
void timing_stop(uint32_t id);
void timing_close(uint32_t id);
uint32_t timing_count_get(uint32_t id);
struct timeval timing_get();
/* uint32_t timing_get(); */
/* float timing_tick2sec(uint32_t ticks); */

float timing_tick2sec(struct timeval start);

#endif /* TIMING_H_ */
