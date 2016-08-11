
#include <stdlib.h>
#include "efti.h"
#include "util.h"
#include "dataset.h"
#include "crossvalid.h"
#include <stdlib.h>
#include <string.h>


#if EFTI_CROSSVALIDATION_SINGLE == 1

#define CROSSVALIDS_NUM	5
#define DATASETS_NUM	1

extern T_Dataset vow_dataset;

T_Dataset*	datasets[DATASETS_NUM] = {
	&vow_dataset,
};
#endif

#if EFTI_CROSSVALIDATION_ALL == 1

#define CROSSVALIDS_NUM	5
#define DATASETS_NUM	35

extern T_Dataset ausc_dataset;
extern T_Dataset bc_dataset;
extern T_Dataset bcw_dataset;
extern T_Dataset ca_dataset;
extern T_Dataset car_dataset;
extern T_Dataset cmc_dataset;
extern T_Dataset ctg_dataset;
extern T_Dataset ger_dataset;
extern T_Dataset gls_dataset;
extern T_Dataset hep_dataset;
extern T_Dataset hrts_dataset;
extern T_Dataset ion_dataset;
extern T_Dataset irs_dataset;
extern T_Dataset jvow_dataset;
extern T_Dataset liv_dataset;
extern T_Dataset lym_dataset;
extern T_Dataset page_dataset;
extern T_Dataset pid_dataset;
extern T_Dataset psd_dataset;
extern T_Dataset sb_dataset;
extern T_Dataset seg_dataset;
extern T_Dataset sick_dataset;
extern T_Dataset son_dataset;
extern T_Dataset spect_dataset;
extern T_Dataset spf_dataset;
extern T_Dataset thy_dataset;
extern T_Dataset ttt_dataset;
extern T_Dataset veh_dataset;
extern T_Dataset vote_dataset;
extern T_Dataset vow_dataset;
extern T_Dataset w21_dataset;
extern T_Dataset w40_dataset;
extern T_Dataset wfr_dataset;
extern T_Dataset wine_dataset;
extern T_Dataset zoo_dataset;

T_Dataset*	datasets[DATASETS_NUM] = {
	&ausc_dataset,
	&bc_dataset,
	&bcw_dataset,
	&ca_dataset,
	&car_dataset,
	&cmc_dataset,
	&ctg_dataset,
	&ger_dataset,
	&gls_dataset,
	&hep_dataset,
	&hrts_dataset,
	&ion_dataset,
	&irs_dataset,
	&jvow_dataset,
	&liv_dataset,
	&lym_dataset,
	&page_dataset,
	&pid_dataset,
	&psd_dataset,
	&sb_dataset,
	&seg_dataset,
	&sick_dataset,
	&son_dataset,
	&spect_dataset,
	&spf_dataset,
	&thy_dataset,
	&ttt_dataset,
	&veh_dataset,
	&vote_dataset,
	&vow_dataset,
	&w21_dataset,
	&w40_dataset,
	&wfr_dataset,
	&wine_dataset,
	&zoo_dataset
};

#endif

int rnd_perm_inst[NUM_INST_MAX];

uint32_t fold_chunk_size;
uint32_t fold_start;

Cv_Status_T cv_stat;

void rnd_perm(int randNos[], int size, unsigned int* seed)
{
    int oneRandno;
    int haveRand[size];
    int i;

    memset(haveRand, 0, size*sizeof(int));

    for (i = 0 ; i < size; i++)
    {
		do
		{
			oneRandno = rand_r(seed) % size;
		} while (haveRand[oneRandno] == 1);
		haveRand[oneRandno] = 1;
		randNos[i] = oneRandno;
    }
    return;
}

Cv_Status_T* crossvalid_init(int ensemble_size, int seed_init, int folds_num, int runs_num)
{
	cv_stat.seed              = seed_init;
    cv_stat.folds_num         = folds_num;
	cv_stat.cur_dataset       = -1;
    cv_stat.datasets_num      = DATASETS_NUM;
    cv_stat.runs_num          = runs_num;
	cv_stat.cur_cs_run        = cv_stat.runs_num-1;
	cv_stat.cur_cs_fold 	  = cv_stat.folds_num-1;
	cv_stat.cur_ensemble_size = ensemble_size;
	cv_stat.cur_ensemble 	  = cv_stat.cur_ensemble_size-1;

	return &cv_stat;
}

Cv_Status_T* crossvalid_next_dataset()
{
	cv_stat.cur_cs_run = 0;
	cv_stat.cur_dataset++;

	if (cv_stat.cur_dataset == DATASETS_NUM)
	{
		return NULL;
	}

	cv_stat.dataset = datasets[cv_stat.cur_dataset];

    efti_printf("$dataset:name=\"%s\",inst_cnt=%d,attr_cnt=%d,categ_max=%d,seed=%d\n", cv_stat.dataset->name, cv_stat.dataset->inst_cnt, cv_stat.dataset->attr_cnt, cv_stat.dataset->categ_max, cv_stat.seed);
    
	return &cv_stat;
}

Cv_Status_T* crossvalid_next_run()
{
	cv_stat.cur_cs_fold = 0;
	cv_stat.cur_cs_run++;

	if (cv_stat.cur_cs_run == cv_stat.runs_num)
	{
		if (!crossvalid_next_dataset()) return NULL;
	}

	rnd_perm(rnd_perm_inst, cv_stat.dataset->inst_cnt, &cv_stat.seed);
	cv_stat.perm = rnd_perm_inst;

	return &cv_stat;
}

Cv_Status_T* crossvalid_next_fold()
{
	cv_stat.cur_ensemble = 0;
	cv_stat.cur_cs_fold++;
	if (cv_stat.cur_cs_fold == cv_stat.folds_num)
	{
		if (!crossvalid_next_run()) return NULL;
	}

	cv_stat.fold_chunk_size = (cv_stat.dataset->inst_cnt + cv_stat.folds_num - 1) / cv_stat.folds_num;
	cv_stat.fold_start = cv_stat.cur_cs_fold*cv_stat.fold_chunk_size;
	if (cv_stat.cur_cs_fold == (cv_stat.folds_num - 1))
	{
		cv_stat.fold_chunk_size -= (cv_stat.fold_chunk_size*cv_stat.folds_num  - cv_stat.dataset->inst_cnt);
	}

	cv_stat.train_set_size = cv_stat.dataset->inst_cnt - cv_stat.fold_chunk_size;

	return &cv_stat;
}

Cv_Status_T* crossvalid_next_ensemble()
{

	cv_stat.cur_ensemble++;
	if (cv_stat.cur_ensemble == cv_stat.cur_ensemble_size)
	{
		if (!crossvalid_next_fold()) return NULL;
	}

	cv_stat.chunk_size  = (cv_stat.train_set_size + cv_stat.cur_ensemble_size - 1) / cv_stat.cur_ensemble_size;
	cv_stat.chunk_start = cv_stat.cur_ensemble*cv_stat.chunk_size;
	cv_stat.chunk_end = cv_stat.chunk_start + cv_stat.chunk_size;

	if (cv_stat.chunk_start > cv_stat.fold_start) {
		cv_stat.chunk_start += cv_stat.fold_chunk_size;
	}

	if (cv_stat.chunk_start + cv_stat.chunk_size > cv_stat.fold_start) {
		cv_stat.chunk_end += cv_stat.fold_chunk_size;
	}

	if (cv_stat.cur_ensemble == (cv_stat.cur_ensemble_size - 1))
	{
		cv_stat.chunk_size -= (cv_stat.chunk_size*cv_stat.cur_ensemble_size - cv_stat.train_set_size);
		cv_stat.chunk_end -= (cv_stat.chunk_size*cv_stat.cur_ensemble_size - cv_stat.train_set_size);
	}

	return &cv_stat;
}

Cv_Status_T* crossvalid_next_conf()
{
	return crossvalid_next_ensemble();
}
