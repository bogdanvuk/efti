/********************************************************************************************************************** 
 *  UCI Dataset
 *  
 **********************************************************************************************************************/
#include <stdint.h>
#include "dataset.h"

#define INST_CNT 150
#define ATTR_CNT 4
#define CATEG_MAX 3

int32_t irs_instances[150][4] = {
    {0x1c71, 0x4fff, 0x08ad, 0x0555},
    {0x1555, 0x3555, 0x08ad, 0x0555},
    {0x0e38, 0x4000, 0x0682, 0x0555},
    {0x0aaa, 0x3aaa, 0x0ad8, 0x0555},
    {0x18e3, 0x5555, 0x08ad, 0x0555},
    {0x271c, 0x6555, 0x0f2f, 0x1000},
    {0x0aaa, 0x4aaa, 0x08ad, 0x0aaa},
    {0x18e3, 0x4aaa, 0x0ad8, 0x0555},
    {0x038e, 0x2fff, 0x08ad, 0x0555},
    {0x1555, 0x3aaa, 0x0ad8, 0x0000},
    {0x271c, 0x5aaa, 0x0ad8, 0x0555},
    {0x11c7, 0x4aaa, 0x0d04, 0x0555},
    {0x11c7, 0x3555, 0x08ad, 0x0000},
    {0x0000, 0x3555, 0x022b, 0x0000},
    {0x3555, 0x6aaa, 0x0456, 0x0555},
    {0x31c7, 0x7fff, 0x0ad8, 0x1000},
    {0x271c, 0x6555, 0x0682, 0x1000},
    {0x1c71, 0x4fff, 0x08ad, 0x0aaa},
    {0x31c7, 0x5fff, 0x0f2f, 0x0aaa},
    {0x1c71, 0x5fff, 0x0ad8, 0x0aaa},
    {0x271c, 0x4aaa, 0x0f2f, 0x0555},
    {0x1c71, 0x5aaa, 0x0ad8, 0x1000},
    {0x0aaa, 0x5555, 0x0000, 0x0555},
    {0x1c71, 0x4555, 0x0f2f, 0x1555},
    {0x11c7, 0x4aaa, 0x1386, 0x0555},
    {0x18e3, 0x3555, 0x0d04, 0x0555},
    {0x18e3, 0x4aaa, 0x0d04, 0x1000},
    {0x2000, 0x4fff, 0x0ad8, 0x0555},
    {0x2000, 0x4aaa, 0x08ad, 0x0555},
    {0x0e38, 0x4000, 0x0d04, 0x0555},
    {0x11c7, 0x3aaa, 0x0d04, 0x0555},
    {0x271c, 0x4aaa, 0x0ad8, 0x1000},
    {0x2000, 0x6fff, 0x0ad8, 0x0000},
    {0x2aaa, 0x7555, 0x08ad, 0x0555},
    {0x1555, 0x3aaa, 0x0ad8, 0x0000},
    {0x18e3, 0x4000, 0x0456, 0x0555},
    {0x2aaa, 0x4fff, 0x0682, 0x0555},
    {0x1555, 0x3aaa, 0x0ad8, 0x0000},
    {0x038e, 0x3555, 0x0682, 0x0555},
    {0x1c71, 0x4aaa, 0x0ad8, 0x0555},
    {0x18e3, 0x4fff, 0x0682, 0x0aaa},
    {0x071c, 0x0fff, 0x0682, 0x0aaa},
    {0x038e, 0x4000, 0x0682, 0x0555},
    {0x18e3, 0x4fff, 0x0d04, 0x1aaa},
    {0x1c71, 0x5fff, 0x1386, 0x1000},
    {0x11c7, 0x3555, 0x08ad, 0x0aaa},
    {0x1c71, 0x5fff, 0x0d04, 0x0555},
    {0x0aaa, 0x4000, 0x08ad, 0x0555},
    {0x238e, 0x5aaa, 0x0ad8, 0x0555},
    {0x18e3, 0x4555, 0x08ad, 0x0555},
    {0x5fff, 0x4000, 0x5045, 0x4555},
    {0x4aaa, 0x4000, 0x4bee, 0x4aaa},
    {0x5c71, 0x3aaa, 0x549c, 0x4aaa},
    {0x2aaa, 0x0fff, 0x4115, 0x4000},
    {0x4e38, 0x2aaa, 0x4e1a, 0x4aaa},
    {0x31c7, 0x2aaa, 0x4bee, 0x4000},
    {0x471c, 0x4555, 0x5045, 0x5000},
    {0x1555, 0x1555, 0x31e5, 0x3000},
    {0x51c7, 0x2fff, 0x4e1a, 0x4000},
    {0x2000, 0x2555, 0x3eea, 0x4555},
    {0x18e3, 0x0000, 0x363c, 0x3000},
    {0x38e3, 0x3555, 0x456c, 0x4aaa},
    {0x3c71, 0x0aaa, 0x4115, 0x3000},
    {0x3fff, 0x2fff, 0x5045, 0x4555},
    {0x2e38, 0x2fff, 0x3868, 0x4000},
    {0x5555, 0x3aaa, 0x49c3, 0x4555},
    {0x2e38, 0x3555, 0x4bee, 0x4aaa},
    {0x3555, 0x2555, 0x4341, 0x3000},
    {0x438e, 0x0aaa, 0x4bee, 0x4aaa},
    {0x2e38, 0x1aaa, 0x3eea, 0x3555},
    {0x38e3, 0x4000, 0x5270, 0x5aaa},
    {0x3fff, 0x2aaa, 0x4115, 0x4000},
    {0x471c, 0x1aaa, 0x549c, 0x4aaa},
    {0x3fff, 0x2aaa, 0x5045, 0x3aaa},
    {0x4aaa, 0x2fff, 0x4797, 0x4000},
    {0x51c7, 0x3555, 0x49c3, 0x4555},
    {0x58e3, 0x2aaa, 0x5270, 0x4555},
    {0x5555, 0x3555, 0x56c7, 0x5555},
    {0x3c71, 0x2fff, 0x4bee, 0x4aaa},
    {0x31c7, 0x2000, 0x363c, 0x3000},
    {0x2aaa, 0x1555, 0x3cbe, 0x3555},
    {0x2aaa, 0x1555, 0x3a93, 0x3000},
    {0x3555, 0x2555, 0x3eea, 0x3aaa},
    {0x3c71, 0x2555, 0x58f2, 0x5000},
    {0x271c, 0x3555, 0x4bee, 0x4aaa},
    {0x3c71, 0x4aaa, 0x4bee, 0x5000},
    {0x5555, 0x3aaa, 0x5045, 0x4aaa},
    {0x471c, 0x0fff, 0x49c3, 0x4000},
    {0x2e38, 0x3555, 0x4341, 0x4000},
    {0x2aaa, 0x1aaa, 0x4115, 0x4000},
    {0x2aaa, 0x2000, 0x49c3, 0x3aaa},
    {0x3fff, 0x3555, 0x4e1a, 0x4555},
    {0x3555, 0x2000, 0x4115, 0x3aaa},
    {0x18e3, 0x0fff, 0x31e5, 0x3000},
    {0x2e38, 0x2555, 0x456c, 0x4000},
    {0x31c7, 0x3555, 0x456c, 0x3aaa},
    {0x31c7, 0x2fff, 0x456c, 0x4000},
    {0x438e, 0x2fff, 0x4797, 0x4000},
    {0x1c71, 0x1aaa, 0x2b63, 0x3555},
    {0x31c7, 0x2aaa, 0x4341, 0x4000},
    {0x471c, 0x4555, 0x6c79, 0x7fff},
    {0x3555, 0x2555, 0x58f2, 0x6000},
    {0x638e, 0x3555, 0x6a4e, 0x6aaa},
    {0x471c, 0x2fff, 0x63cb, 0x5aaa},
    {0x4e38, 0x3555, 0x6822, 0x7000},
    {0x7555, 0x3555, 0x797d, 0x6aaa},
    {0x1555, 0x1aaa, 0x4bee, 0x5555},
    {0x6aaa, 0x2fff, 0x72fb, 0x5aaa},
    {0x5555, 0x1aaa, 0x6822, 0x5aaa},
    {0x671c, 0x5555, 0x6ea4, 0x7fff},
    {0x4e38, 0x4000, 0x58f2, 0x6555},
    {0x4aaa, 0x2555, 0x5d49, 0x6000},
    {0x58e3, 0x3555, 0x61a0, 0x6aaa},
    {0x31c7, 0x1aaa, 0x56c7, 0x6555},
    {0x3555, 0x2aaa, 0x58f2, 0x7aaa},
    {0x4aaa, 0x4000, 0x5d49, 0x7555},
    {0x4e38, 0x3555, 0x61a0, 0x5aaa},
    {0x78e3, 0x5fff, 0x7ba9, 0x7000},
    {0x78e3, 0x2000, 0x7fff, 0x7555},
    {0x3c71, 0x0aaa, 0x56c7, 0x4aaa},
    {0x5c71, 0x4000, 0x65f7, 0x7555},
    {0x2e38, 0x2aaa, 0x549c, 0x6555},
    {0x78e3, 0x2aaa, 0x7ba9, 0x6555},
    {0x471c, 0x2555, 0x549c, 0x5aaa},
    {0x5555, 0x4555, 0x65f7, 0x6aaa},
    {0x671c, 0x4000, 0x6c79, 0x5aaa},
    {0x438e, 0x2aaa, 0x5270, 0x5aaa},
    {0x3fff, 0x3555, 0x549c, 0x5aaa},
    {0x4aaa, 0x2aaa, 0x63cb, 0x6aaa},
    {0x671c, 0x3555, 0x6822, 0x5000},
    {0x6e38, 0x2aaa, 0x6ea4, 0x6000},
    {0x7fff, 0x5fff, 0x7527, 0x6555},
    {0x4aaa, 0x2aaa, 0x63cb, 0x7000},
    {0x471c, 0x2aaa, 0x58f2, 0x4aaa},
    {0x3fff, 0x2000, 0x63cb, 0x4555},
    {0x78e3, 0x3555, 0x6ea4, 0x7555},
    {0x471c, 0x4aaa, 0x63cb, 0x7aaa},
    {0x4aaa, 0x3aaa, 0x61a0, 0x5aaa},
    {0x3c71, 0x3555, 0x5270, 0x5aaa},
    {0x5c71, 0x3aaa, 0x5f75, 0x6aaa},
    {0x5555, 0x3aaa, 0x63cb, 0x7aaa},
    {0x5c71, 0x3aaa, 0x58f2, 0x7555},
    {0x3555, 0x2555, 0x58f2, 0x6000},
    {0x58e3, 0x4000, 0x6a4e, 0x7555},
    {0x5555, 0x4555, 0x65f7, 0x7fff},
    {0x5555, 0x3555, 0x5b1e, 0x7555},
    {0x471c, 0x1aaa, 0x56c7, 0x6000},
    {0x4e38, 0x3555, 0x5b1e, 0x6555},
    {0x438e, 0x4aaa, 0x5f75, 0x7555},
    {0x38e3, 0x3555, 0x58f2, 0x5aaa}
};


uint32_t irs_classes[150] = {
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03
};

T_Dataset irs_dataset = {
    INST_CNT,
    ATTR_CNT,
    CATEG_MAX,
    "irs",
    (int32_t*) irs_instances,
    irs_classes
};
