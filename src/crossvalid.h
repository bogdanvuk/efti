/*
 * crossvalid.h
 *
 *  Created on: Jul 3, 2015
 *      Author: bvukobratovic
 */

#ifndef __CROSSVALID_H_
#define __CROSSVALID_H_

#include "dataset.h"
#include <stdlib.h>

#define EFTI_CROSSVALIDATION		0
#define EFTI_CROSSVALIDATION_SINGLE	0
#define EFTI_CROSSVALIDATION_ALL 1

typedef struct{
    T_Dataset* dataset;
    char* dselect;
    int cur_dataset;
    int datasets_num;
    int runs_num;
    int cur_cs_run;
    int cur_cs_fold;
    int folds_num;
    int cur_ensemble_size;
    int cur_ensemble;
    uint32_t chunk_size;
    uint32_t chunk_start;
    uint32_t chunk_end;
    uint32_t fold_chunk_size;
    uint32_t fold_start;
    uint32_t train_set_size;
    int* perm;
    unsigned int seed;
}Cv_Status_T;

Cv_Status_T* crossvalid_next_conf();

Cv_Status_T* crossvalid_init(char* dselect, int ensemble_size, int seed_init, 
                             int folds_num, int runs_num);
#endif /* CROSSVALID_H_ */
