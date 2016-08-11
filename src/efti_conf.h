/*
 * EFTI_conf.h
 *
 *  Created on: Feb 20, 2015
 *      Author: bvukobratovic
 */

#ifndef EFTI_CONF_H_
#define EFTI_CONF_H_

#define EFTI_PC 		1
#define EFTI_PROFILING	0

#if EFTI_PC == 0

#define EFTI_HW						0
#define EFTI_HW_SW_FITNESS			0
#define EFTI_SW						1

#else

#define EFTI_HW						0
#define EFTI_HW_SW_FITNESS			0
#define EFTI_SW						1

#endif

#endif /* EFTI_CONF_H_ */
