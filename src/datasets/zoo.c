/********************************************************************************************************************** 
 *  UCI Dataset
 *  
 **********************************************************************************************************************/
#include <stdint.h>
#include "dataset.h"

#define INST_CNT 101
#define ATTR_CNT 17
#define CATEG_MAX 7

int32_t zoo_instances[101][17] = {
    {0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x7fff},
    {0x014a, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x0295, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x03e0, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x7fff},
    {0x052b, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x0676, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x07c1, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x7fff, 0x7fff},
    {0x090c, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000},
    {0x0a57, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x0ba2, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x0000, 0x7fff, 0x0000},
    {0x0ced, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x0e38, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x7fff, 0x0000},
    {0x0f83, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x10ce, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x1219, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x0000},
    {0x1364, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x14af, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x15fa, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x1745, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff},
    {0x1890, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff},
    {0x19db, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x7fff, 0x0000},
    {0x1b26, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x1c71, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x1dbc, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x1f07, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x2052, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x0000},
    {0x2052, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x4000, 0x0000, 0x0000, 0x0000},
    {0x219d, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x22e8, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x2433, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x0000, 0x7fff, 0x7fff},
    {0x257e, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x26c9, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x7fff, 0x7fff},
    {0x2814, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x0000, 0x0000, 0x7fff},
    {0x295f, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x2aaa, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x2bf5, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x7fff, 0x0000},
    {0x2d40, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x0000},
    {0x2e8b, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x2fd6, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x3121, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x6000, 0x0000, 0x7fff, 0x0000},
    {0x326c, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x33b7, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x3502, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x364d, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x3798, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x38e3, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x3a2e, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x3b79, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x3cc4, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x3e0f, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x0000},
    {0x3f5a, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x40a5, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x41f0, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x0000},
    {0x433b, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff},
    {0x4486, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x0000},
    {0x45d1, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x471c, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x4867, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x7fff, 0x0000},
    {0x49b2, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x4afd, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x4c48, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff},
    {0x4d93, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x4ede, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x5029, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x5174, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x52bf, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x7fff, 0x7fff},
    {0x540a, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff},
    {0x5555, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x56a0, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x7fff, 0x7fff},
    {0x57eb, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x5936, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x7fff, 0x7fff},
    {0x5a81, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x5bcc, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000},
    {0x5d17, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x5e62, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff},
    {0x5fad, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x60f8, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x6243, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x638e, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x64d9, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x6624, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x676f, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x68ba, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000},
    {0x6a05, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x6b50, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x6c9b, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x5000, 0x0000, 0x0000, 0x0000},
    {0x6de6, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff},
    {0x6f31, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x707c, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x71c7, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x0000, 0x0000, 0x0000},
    {0x7312, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x745d, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x0000},
    {0x75a8, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff},
    {0x76f3, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000},
    {0x783e, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x0000},
    {0x7989, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x7ad4, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x7fff},
    {0x7c1f, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x6000, 0x0000, 0x0000, 0x0000},
    {0x7d6a, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x4000, 0x7fff, 0x0000, 0x7fff},
    {0x7eb5, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
    {0x7fff, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x7fff, 0x0000, 0x0000, 0x0000, 0x7fff, 0x7fff, 0x0000, 0x0000, 0x2000, 0x7fff, 0x0000, 0x0000}
};


uint32_t zoo_classes[101] = {
    0x06, 0x06, 0x03, 0x06, 0x06, 0x06, 0x06, 0x03, 0x03, 0x06, 0x06, 0x02, 0x03, 0x05, 0x05, 0x05,
    0x02, 0x06, 0x03, 0x06, 0x02, 0x02, 0x06, 0x02, 0x04, 0x01, 0x01, 0x06, 0x06, 0x06, 0x04, 0x06,
    0x06, 0x02, 0x03, 0x06, 0x06, 0x02, 0x03, 0x04, 0x04, 0x02, 0x04, 0x02, 0x06, 0x06, 0x05, 0x06,
    0x06, 0x06, 0x06, 0x04, 0x01, 0x05, 0x06, 0x06, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x07, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x02, 0x05, 0x03, 0x06, 0x06, 0x07, 0x05, 0x02, 0x02,
    0x07, 0x05, 0x03, 0x02, 0x06, 0x05, 0x03, 0x02, 0x04, 0x01, 0x07, 0x07, 0x03, 0x06, 0x06, 0x02,
    0x06, 0x04, 0x06, 0x05, 0x02
};

T_Dataset zoo_dataset = {
    INST_CNT,
    ATTR_CNT,
    CATEG_MAX,
    "zoo",
    (int32_t*) zoo_instances,
    zoo_classes
};
