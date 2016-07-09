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
#include "dt2js.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define EFTI_CROSSVALIDATION			1
#define EFTI_CROSSVALIDATION_SINGLE		0
#define EFTI_CROSSVALIDATION_ALL		1
#define EFTI_CROSSVALIDATION_RUNS		5
#define MAX_ITERATIONS 			1000000
#define SEED					29

Efti_Conf_t efti_config = {
	MAX_ITERATIONS,	// max_iterations
	0.002,  // topology_mutation_rate;
	0.01, 	// weights_mutation_rate;
	0.01, 	// search_probability;
	0.001, 	// search_probability_raise_due_to_stagnation_step;
	0.00005, // topo_mutation_rate_raise_due_to_stagnation_step;
	0.00004, // weight_mutation_rate_raise_due_to_stagnation_step;
	5e-7,  // return_to_best_prob_iteration_increment;
	0.2,   // complexity_weight;
	0.2,   // impurity_weight;
    1,     // use_impurity_topo_mut;
	0     // use_impurity_weight_mut;
};

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
#define DATASETS_NUM	20

extern T_Dataset ausc_dataset;
extern T_Dataset bc_dataset;
extern T_Dataset bcw_dataset;
extern T_Dataset ger_dataset;
extern T_Dataset gls_dataset;
extern T_Dataset hep_dataset;
extern T_Dataset hrts_dataset;
extern T_Dataset ion_dataset;
extern T_Dataset irs_dataset;
extern T_Dataset liv_dataset;
extern T_Dataset lym_dataset;
extern T_Dataset page_dataset;
extern T_Dataset pid_dataset;
extern T_Dataset son_dataset;
extern T_Dataset thy_dataset;
extern T_Dataset veh_dataset;
extern T_Dataset vote_dataset;
extern T_Dataset vow_dataset;
extern T_Dataset w40_dataset;
extern T_Dataset zoo_dataset;

T_Dataset*	datasets[DATASETS_NUM] = {
	&ausc_dataset,
	&bc_dataset,
	&bcw_dataset,
	&ger_dataset,
	&gls_dataset,
	&hep_dataset,
	&hrts_dataset,
	&ion_dataset,
	&irs_dataset,
	&liv_dataset,
	&lym_dataset,
	&page_dataset,
	&pid_dataset,
	&son_dataset,
	&thy_dataset,
	&veh_dataset,
	&vote_dataset,
	&vow_dataset,
	&w40_dataset,
	&zoo_dataset
};

//#define CROSSVALIDS_NUM	5
//#define DATASETS_NUM	22
//
//extern T_Dataset ausc_dataset;
//extern T_Dataset ca_dataset;
//extern T_Dataset car_dataset;
//extern T_Dataset cmc_dataset;
//extern T_Dataset ctg_dataset;
//extern T_Dataset ger_dataset;
//extern T_Dataset jvow_dataset;
//extern T_Dataset page_dataset;
//extern T_Dataset pid_dataset;
//extern T_Dataset psd_dataset;
//extern T_Dataset sb_dataset;
//extern T_Dataset seg_dataset;
//extern T_Dataset sick_dataset;
//extern T_Dataset spect_dataset;
//extern T_Dataset spf_dataset;
//extern T_Dataset thy_dataset;
//extern T_Dataset veh_dataset;
//extern T_Dataset vote_dataset;
//extern T_Dataset vow_dataset;
//extern T_Dataset w21_dataset;
//extern T_Dataset wfr_dataset;
//extern T_Dataset wine_dataset;
//
//T_Dataset*	datasets[DATASETS_NUM] = {
//	&ausc_dataset,
//	&ca_dataset,
//	&car_dataset,
//	&cmc_dataset,
//	&ctg_dataset,
//	&ger_dataset,
//	&jvow_dataset,
//	&page_dataset,
//	&pid_dataset,
//	&psd_dataset,
//	&sb_dataset,
//	&seg_dataset,
//	&sick_dataset,
//	&spect_dataset,
//	&spf_dataset,
//	&thy_dataset,
//	&veh_dataset,
//	&vote_dataset,
//	&vow_dataset,
//	&w21_dataset,
//	&wfr_dataset,
//	&wine_dataset
//};
#endif

#if CROSSVALIDS_NUM == 1
	int rnd_perm_inst[NUM_INST_MAX];
#else
	int rnd_perm_inst[NUM_INST_MAX / (CROSSVALIDS_NUM - 1)*CROSSVALIDS_NUM];
#endif

unsigned int seed;

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
			oneRandno = rand_r(&seed) % size;
		} while (haveRand[oneRandno] == 1);
		haveRand[oneRandno] = 1;
		randNos[i] = oneRandno;
    }
    return;
}

int crossvalidation()
{
	uint32_t i, j, k, n;
	tree_node* dt;
	float fitness;
	float accuracy;
	uint32_t leaves;
	uint32_t nonleaves;
	float t_hb;

//	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
//					(Xil_ExceptionHandler)MyDataAbortHandler,
//					NULL);


	seed = SEED;

	efti_init();

#if (EFTI_CROSSVALIDATION == 1)
	efti_printf("$efti_config:max_iterations=%d,topology_mutation_rate=%e,weights_mutation_rate=%e,search_probability=%e,search_probability_raise_due_to_stagnation_step=%e,weight_mutation_rate_raise_due_to_stagnation_step=%e,return_to_best_prob_iteration_increment=%e,complexity_weight=%e,seed=%d\n",
			efti_config.max_iterations,
			efti_config.topology_mutation_rate,
			efti_config.weights_mutation_rate,
			efti_config.search_probability,
			efti_config.search_probability_raise_due_to_stagnation_step,
			efti_config.weight_mutation_rate_raise_due_to_stagnation_step,
			efti_config.return_to_best_prob_iteration_increment,
			efti_config.complexity_weight,
			SEED
			);

	for (i = 0; i < DATASETS_NUM; i++)
	{
		for (n=0; n < EFTI_CROSSVALIDATION_RUNS; n++)
		{

			efti_printf("$dataset:name=\"%s\",inst_cnt=%d,attr_cnt=%d,categ_max=%d,seed=%d\n", datasets[i]->name, datasets[i]->inst_cnt, datasets[i]->attr_cnt, datasets[i]->categ_max,seed);

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

				efti_reset(&efti_config, datasets[i]);

				for (j = 0; j < datasets[i]->inst_cnt; j++)
				{
					if ((j < k*crossvalid_chunk) || (j >= (k+1)*crossvalid_chunk))
					{
						train_inst_cnt++;
						efti_load_instance(&datasets[i]->instances[datasets[i]->attr_cnt*rnd_perm_inst[j]], datasets[i]->classes[rnd_perm_inst[j]]);
					}
				}

				dt = efti(&fitness, &leaves, &nonleaves, &t_hb, &seed);

	#if EFTI_PROFILING == 0

				uint32_t end_inst = (k+1)*crossvalid_chunk > datasets[i]->inst_cnt ? datasets[i]->inst_cnt : (k+1)*crossvalid_chunk;

				for (j = k*crossvalid_chunk; j < end_inst; j++)
				{
					test_inst_cnt++;
					efti_load_instance(&datasets[i]->instances[datasets[i]->attr_cnt*rnd_perm_inst[j]], datasets[i]->classes[rnd_perm_inst[j]]);
				}

				accuracy = efti_eval(dt);

				efti_printf("$cv_pc_run:dataset=\"%s\",run=%d,id=%d,train_cnt=%d,test_cnt=%d,fitness=%f,accuracy=%f,leaves=%d,nonleaves=%d,time=%f\n",
						datasets[i]->name,
						n,
						k,
						train_inst_cnt,
						test_inst_cnt,
						fitness,
						accuracy,
						leaves,
						nonleaves,
						t_hb
						);
	#endif

				efti_reset(&efti_config, datasets[i]);
			}
		}
	}

#endif

	efti_close();

	return 0;
}

/** Program to calculate the area and perimeter of
 * a rectangle using command line arguments
 */
void print_usage() {
    printf("Usage: rectangle [ap] -l num -b num\n");
}

int main(int argc, char *argv[]) {
    int opt= 0;

    //Specifying the expected options
    //The two options l and b expect numbers as argument
    static struct option long_options[] = {
		{"max_iter",  optional_argument, 0,  'm' },
        {"topo_mut",  optional_argument, 0,  't' },
		{"weight_mut",  optional_argument, 0,  'w' },
		{"search_prob",  optional_argument, 0,  's' },
		{"s_accel_stagn",  optional_argument, 0,  'x' },
		{"t_accel_stagn",  optional_argument, 0,  'y' },
		{"w_accel_stagn",  optional_argument, 0,  'z' },
		{"return_prob",  optional_argument, 0,  'r' },
		{"oversize_w",  optional_argument, 0,  'o' },
        {0,           0,                 0,  0   }
    };

    int long_index =0;
    while ((opt = getopt_long(argc, argv,"m::t::w::s::x::y::z::r::o::",
                   long_options, &long_index )) != -1) {
        switch (opt) {
             case 'm' : efti_config.max_iterations = atoi(optarg);
                 break;
             case 't' : efti_config.topology_mutation_rate = atof(optarg);
                 break;
             case 'w' : efti_config.weights_mutation_rate = atof(optarg);
                 break;
             case 's' : efti_config.search_probability = atof(optarg);
                 break;
             case 'x' : efti_config.search_probability_raise_due_to_stagnation_step = atof(optarg);
                 break;
             case 'y' : efti_config.topo_mutation_rate_raise_due_to_stagnation_step = atof(optarg);
                 break;
             case 'z' : efti_config.weight_mutation_rate_raise_due_to_stagnation_step = atof(optarg);
                 break;
             case 'r' : efti_config.return_to_best_prob_iteration_increment = atof(optarg);
                 break;
             case 'o' : efti_config.complexity_weight = atof(optarg);
                 break;
             default: print_usage();
                 exit(EXIT_FAILURE);
        }
    }

    return crossvalidation();
}
