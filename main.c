/*
 * Empty C++ Application
 */

#include <stdlib.h>
#include <stdio.h>
#include "efti_conf.h"
#include "efti.h"
#include "timing.h"
#include "hw_config.h"
#include "util.h"
#include "dataset.h"
#include <string.h>

#define EFTI_CROSSVALIDATION			1
#define EFTI_CROSSVALIDATION_SINGLE		1
#define EFTI_CROSSVALIDATION_ALL		0
#define EFTI_CROSSVALIDATION_RUNS		1
#define MAX_ITERATIONS 			500000
#define SEED					29

Efti_Conf_t efti_config = {
	MAX_ITERATIONS,	// max_iterations
	0.003,  // topology_mutation_rate;
	0.02, 	// weights_mutation_rate;
	0.001, 	// search_probability;
	0.001, 	// search_probability_raise_due_to_stagnation_step;
	0.0001, // topo_mutation_rate_raise_due_to_stagnation_step;
	0.0001, // weight_mutation_rate_raise_due_to_stagnation_step;
	5e-11,  // return_to_best_prob_iteration_increment;
	0.01,   // complexity_weight;
};

#if EFTI_CROSSVALIDATION_SINGLE == 1

#define CROSSVALIDS_NUM	1
#define DATASETS_NUM	1

extern T_Dataset vene_dataset;

T_Dataset*	datasets[DATASETS_NUM] = {
	&vene_dataset,
};
#endif

#if EFTI_CROSSVALIDATION_ALL == 1

#define CROSSVALIDS_NUM	5
#define DATASETS_NUM	22

extern T_Dataset ausc_dataset;
extern T_Dataset ca_dataset;
extern T_Dataset car_dataset;
extern T_Dataset cmc_dataset;
extern T_Dataset ctg_dataset;
extern T_Dataset ger_dataset;
extern T_Dataset jvow_dataset;
extern T_Dataset page_dataset;
extern T_Dataset pid_dataset;
extern T_Dataset psd_dataset;
extern T_Dataset sb_dataset;
extern T_Dataset seg_dataset;
extern T_Dataset sick_dataset;
extern T_Dataset spect_dataset;
extern T_Dataset spf_dataset;
extern T_Dataset thy_dataset;
extern T_Dataset veh_dataset;
extern T_Dataset vote_dataset;
extern T_Dataset vow_dataset;
extern T_Dataset w21_dataset;
extern T_Dataset wfr_dataset;
extern T_Dataset wine_dataset;

T_Dataset*	datasets[DATASETS_NUM] = {
	&ausc_dataset,
	&ca_dataset,
	&car_dataset,
	&cmc_dataset,
	&ctg_dataset,
	&ger_dataset,
	&jvow_dataset,
	&page_dataset,
	&pid_dataset,
	&psd_dataset,
	&sb_dataset,
	&seg_dataset,
	&sick_dataset,
	&spect_dataset,
	&spf_dataset,
	&thy_dataset,
	&veh_dataset,
	&vote_dataset,
	&vow_dataset,
	&w21_dataset,
	&wfr_dataset,
	&wine_dataset
};
#endif

#if CROSSVALIDS_NUM == 1
	int rnd_perm_inst[NUM_INST_MAX];
#else
	int rnd_perm_inst[NUM_INST_MAX / (CROSSVALIDS_NUM - 1)*CROSSVALIDS_NUM];
#endif


//#include "xil_exception.h"
//volatile u32 DFSR;
//
//void MyDataAbortHandler()
//{
//	DFSR = mfcp(XREG_CP15_DATA_FAULT_STATUS);
//
//}

void rnd_perm(int randNos[], int size)
{
    int oneRandno;
    int haveRand[size];
    int i;

    memset(haveRand, 0, size*sizeof(int));

    for (i = 0 ; i < size; i++)
    {
		do
		{
			oneRandno = rand() % size;
		} while (haveRand[oneRandno] == 1);
		haveRand[oneRandno] = 1;
		randNos[i] = oneRandno;
    }
    return;
}

int find_node_id(tree_node* n, tree_node* ids[], int ids_cnt) {
	int i;
	for (i = 0; i < ids_cnt; i++) {
		if (ids[i] == n) {
			return i;
		}
	}
	return 0;
}

void _dt2json_rec(char** js, tree_node* dt, T_Dataset* ds, tree_node* ids[], int ids_cnt){
	const char *fmt = "\"%d\": {\"lvl\": %d, "
						       	"\"id\": %d,"
							    "\"cls\": %d,"
							    "\"left\": \"%d\","
							    "\"right\": \"%d\","
							    "\"thr\": %0.5f,"
							    "\"coeffs\": [%s]},";

	int left = -1;
	int right = -1;
	char coeffs[256]= "";
	char *coef_cur;
	int cls = 0;
	int i;
	float thr = 0;

	if (dt->left) {
		left = find_node_id(dt->left, ids, ids_cnt);
	}

	if (dt->right) {
		right = find_node_id(dt->right, ids, ids_cnt);
	}

	if (!(dt->left) && !(dt->right)) {
		cls = dt->id;
	} else {
		thr = ((float) (dt->weights[NUM_ATTRIBUTES] << (DT_ADDER_TREE_DEPTH + 1))) / (1 << 15);
		coef_cur = coeffs;
		for (i = 0; i < ds->attr_cnt; i++) {
			if (i != 0) {
				*coef_cur = ',';
				coef_cur++;
			}

			coef_cur += sprintf(coef_cur, "%0.5f", ((float) dt->weights[i]) / (1 << 15));
		}
	}

	int id = find_node_id(dt, ids, ids_cnt);
	*js += sprintf(*js, fmt, id,
							dt->level,
							id,
							cls,
							left,
							right,
							thr,
							coeffs
			);

	if (dt->left) {
		_dt2json_rec(js, dt->left, ds, ids, ids_cnt);
		_dt2json_rec(js, dt->right, ds, ids, ids_cnt);
	}

}

void dt2json(char** js, tree_node* dt, T_Dataset* ds){
	tree_node* ids[1 << MAX_TREE_DEPTH];
	tree_node* cur;
	int ids_cnt = 0;
	int nodes_processed = 0;

	ids[ids_cnt++] = dt;
	while (ids_cnt > nodes_processed) {
		cur = ids[nodes_processed];
		if (cur->left) {
			ids[ids_cnt++] = cur->left;
			ids[ids_cnt++] = cur->right;
		}
		nodes_processed++;
	}

	_dt2json_rec(js, dt, ds, ids, ids_cnt);
}

char json_buff[2048];

void dump_dt2json(char* fn, tree_node* dt, T_Dataset* ds) {
	char *js;
	json_buff[0] = '{';
	js = &json_buff[1];
	dt2json(&js, dt, ds);

	json_buff[strlen(json_buff) - 1] = 0; //Remove last ',' in string
	strcat(json_buff, "}");

	FILE *fp = fopen(fn, "w");
    if (fp != NULL)
    {
        fputs(json_buff, fp);
        fclose(fp);
    }
}

int main()
{
	uint32_t i, j, k, n;
	tree_node* dt;
	float fitness;
	float accuracy;
	float tot_reclass;
	float avg_tot_reclass;
	uint32_t leaves;
	uint32_t nonleaves;
	float t_fitness_calc_avg;
	float t_hb;

//	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
//					(Xil_ExceptionHandler)MyDataAbortHandler,
//					NULL);

	srand (SEED);

	efti_init();

#if (EFTI_CROSSVALIDATION == 1)
	efti_printf("$efti_config:max_iterations=%d,topology_mutation_rate=%e,weights_mutation_rate=%e,search_probability=%e,search_probability_raise_due_to_stagnation_step=%e,weight_mutation_rate_raise_due_to_stagnation_step=%e,return_to_best_prob_iteration_increment=%e,complexity_weight=%e\n",
			efti_config.max_iterations,
			efti_config.topology_mutation_rate,
			efti_config.weights_mutation_rate,
			efti_config.search_probability,
			efti_config.search_probability_raise_due_to_stagnation_step,
			efti_config.weight_mutation_rate_raise_due_to_stagnation_step,
			efti_config.return_to_best_prob_iteration_increment,
			efti_config.complexity_weight
			);

	for (i = 0; i < DATASETS_NUM; i++)
	{
		avg_tot_reclass = 0;
		for (n=0; n < EFTI_CROSSVALIDATION_RUNS; n++)
		{

			efti_printf("$dataset:name=%s,inst_cnt=%d,attr_cnt=%d,categ_max=%d\n", datasets[i]->name, datasets[i]->inst_cnt, datasets[i]->attr_cnt, datasets[i]->categ_max);

			rnd_perm(rnd_perm_inst, datasets[i]->inst_cnt);

	#if CROSSVALIDS_NUM == 1
			uint32_t crossvalid_chunk = 0;
	#else
			uint32_t crossvalid_chunk = (datasets[i]->inst_cnt + CROSSVALIDS_NUM - 1) / CROSSVALIDS_NUM;
	#endif

			for (k = 0; k < CROSSVALIDS_NUM; k++)
			{
				uint32_t train_inst_cnt = 0;
				uint32_t test_inst_cnt = 0;

				efti_reset(&efti_config, datasets[i]->attr_cnt, datasets[i]->categ_max);

				for (j = 0; j < datasets[i]->inst_cnt; j++)
				{
					if ((j < k*crossvalid_chunk) || (j >= (k+1)*crossvalid_chunk))
					{
						train_inst_cnt++;
						efti_load_instance(&datasets[i]->instances[datasets[i]->attr_cnt*rnd_perm_inst[j]], datasets[i]->classes[rnd_perm_inst[j]]);
					}
				}

				dt = efti(&fitness, &leaves, &nonleaves, &t_hb, &t_fitness_calc_avg, &tot_reclass);
				dump_dt2json("dt.js", dt, datasets[i]);
				avg_tot_reclass += tot_reclass;

				efti_reset(&efti_config, datasets[i]->attr_cnt, datasets[i]->categ_max);

	#if EFTI_PROFILING == 0

				uint32_t end_inst = (k+1)*crossvalid_chunk > datasets[i]->inst_cnt ? datasets[i]->inst_cnt : (k+1)*crossvalid_chunk;

				for (j = k*crossvalid_chunk; j < end_inst; j++)
				{
					test_inst_cnt++;
					efti_load_instance(&datasets[i]->instances[datasets[i]->attr_cnt*rnd_perm_inst[j]], datasets[i]->classes[rnd_perm_inst[j]]);
				}

				accuracy = efti_eval(dt);

				efti_printf("$cv_run:dataset=%s,run=%d,id=%d,seed=%d,train_cnt=%d,test_cnt=%d,fitness=%f,accuracy=%f,leaves=%d,nonleaves=%d,timing=%f,fitness_calc_cycle_timing=%e\n",
						datasets[i]->name,
						n,
						k,
						SEED,
						train_inst_cnt,
						test_inst_cnt,
						fitness,
						accuracy,
						leaves,
						nonleaves,
						t_hb,
						t_fitness_calc_avg
						);
	#endif
			}
		}
		efti_printf("!!Average total different classifications: %f\n", avg_tot_reclass);
	}

#endif

//	efti_reset(&efti_config, bc_dataset.attr_cnt, bc_dataset.categ_max);
//
//	for (i = 0; i < bc_dataset.inst_cnt; i++)
//	{
//		efti_load_instance(&bc_dataset.instances[bc_dataset.attr_cnt*i], bc_dataset.classes[i]);
//	}
//
//	dt = efti_run(&fitness, &leaves, &nonleaves, &t_hb, &t_fitness_calc_avg);

//	efti_reset(&efti_config, VEH_ATTR_CNT, VEH_CATEG_MAX);
//
//	for (i = 0; i < VEH_INST_CNT; i++)
//	{
//		efti_load_instance(veh_instances[i], veh_categories[i]);
//	}
//
//	dt = efti_run(&fitness, &leaves, &nonleaves, &t_hb, &t_fitness_calc_avg);

//	efti_reset(&efti_config, CAR_ATTR_CNT, CAR_CATEG_MAX);
//
//	for (i = 0; i < CAR_INST_CNT; i++)
//	{
//		efti_load_instance(car_instances[i], car_categories[i]);
//	}

//	dt = efti_run(&fitness, &leaves, &nonleaves, &t_hb, &t_fitness_calc_avg);

	efti_close();

//	while(1)
//	{}

	return 0;
}
