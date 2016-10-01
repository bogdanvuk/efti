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
#include "crossvalid.h"

#define CROSSVALIDS_NUM           5
#define EFTI_CROSSVALIDATION_RUNS 5
#define ENSEMBLE_SIZE_MAX         32
#define MAX_ITERATIONS            50000
#define SEED                      29

void sgenrand(unsigned long seed);

Efti_Conf_t efti_config = {
    MAX_ITERATIONS,	// max_iterations
    0.5,          // topology_mutation_rate;
    0.0,           // weights_mutation_rate;
    0.0,           // search_probability;
    0.0,          // search_probability_raise_due_to_stagnation_step;
    0.0,        // topo_mutation_rate_raise_due_to_stagnation_step;
    0.0,        // weight_mutation_rate_raise_due_to_stagnation_step;
    0.0,           // return_to_best_prob_iteration_increment;
    0.02,            // complexity_weight;
    0,              // impurity_weight;
    0,              // use_impurity_topo_mut;
    0,              // use_impurity_weight_mut;
    1,              // ensemble_size
    NULL            // dataset_list
};

int load_dataset_to_efti(T_Dataset* ds, int* perm, int start, int end, int ex_start, int ex_end){
    int j;
    int total_cnt = 0;
    
    for (j = start; j < end ; j++)
    {
        if ((j < ex_start) || (j >= ex_end))
        {
            efti_load_instance(&ds->instances[ds->attr_cnt*perm[j]],
                               ds->classes[perm[j]]);
            total_cnt++;
        }
    }
    return total_cnt;
}

int crossvalidation()
{
    uint32_t i, j, k, n, e;
    tree_node* dt[ENSEMBLE_SIZE_MAX];
    float fitness;
    float avg_fit;
    float avg_size;
    float accuracy;
    uint32_t leaves;
    uint32_t nonleaves;
    float t_hb;
    int train_num;
    Cv_Status_T* cv_conf;

//	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
//					(Xil_ExceptionHandler)MyDataAbortHandler,
//					NULL);


    efti_init();
    sgenrand(SEED);

    efti_printf("$efti_config:max_iterations=%d,topology_mutation_rate=%e,"
                "topo_mutation_rate_raise_due_to_stagnation_step=%e,"
                "weights_mutation_rate=%e,search_probability=%e,search_probability_raise_due_to_stagnation_step=%e,"
                "weight_mutation_rate_raise_due_to_stagnation_step=%e,return_to_best_prob_iteration_increment=%e,"
                "complexity_weight=%e,impurity_weight=%e,use_impurity_topo_mut=%d,use_impurity_weight_mut=%d,"
                "ensemble_size=%d,seed=%d\n",
                efti_config.max_iterations,
                efti_config.topology_mutation_rate,
                efti_config.topo_mutation_rate_raise_due_to_stagnation_step,
                efti_config.weights_mutation_rate,
                efti_config.search_probability,
                efti_config.search_probability_raise_due_to_stagnation_step,
                efti_config.weight_mutation_rate_raise_due_to_stagnation_step,
                efti_config.return_to_best_prob_iteration_increment,
                efti_config.complexity_weight,
                efti_config.impurity_weight,
                efti_config.use_impurity_topo_mut,
                efti_config.use_impurity_weight_mut,
                efti_config.ensemble_size,
                SEED
        );

    cv_conf = crossvalid_init(efti_config.dataset_selection, efti_config.ensemble_size, SEED, CROSSVALIDS_NUM, EFTI_CROSSVALIDATION_RUNS);
    
    for (i = 0; i < cv_conf->datasets_num; i++)
    {
        for (n=0; n < EFTI_CROSSVALIDATION_RUNS; n++)
        {

            for (k = 0; k < CROSSVALIDS_NUM; k++)
            {
                avg_fit = 0;
                avg_size = 0;
                for (e = 0; e < efti_config.ensemble_size; e++)
                {
                    cv_conf = crossvalid_next_conf();
                
                    efti_reset(&efti_config, cv_conf->dataset);
                    train_num = load_dataset_to_efti(cv_conf->dataset, cv_conf->perm,
                                                     cv_conf->chunk_start, cv_conf->chunk_end,
                                                     cv_conf->fold_start, cv_conf->fold_start + cv_conf->fold_chunk_size);
                    dt[e] = efti(&fitness, &leaves, &nonleaves, &t_hb, &cv_conf->seed);
                    avg_fit += fitness;
                    avg_size += nonleaves;
                }

#if EFTI_PROFILING == 0

                load_dataset_to_efti(cv_conf->dataset, cv_conf->perm,
                                     cv_conf->fold_start, cv_conf->fold_start + cv_conf->fold_chunk_size,
                                     0, 0);
                accuracy =  ensemble_eval(dt, efti_config.ensemble_size);
                efti_printf("$cv_pc_run:dataset=\"%s\",run=%d,id=%d,train_range=(%d,%d),"
                            "train_cnt=%d,test_range=(%d,%d),test_cnt=%d,fitness=%f,accuracy=%f,"
                            "leaves=%d,nonleaves=%f,time=%f,ensemble_size=%d\n",
                            cv_conf->dataset->name,
                            n,
                            k,
                            cv_conf->chunk_start,
                            cv_conf->chunk_end,
                            cv_conf->chunk_size,
                            cv_conf->fold_start,
                            cv_conf->fold_start + cv_conf->fold_chunk_size,
                            cv_conf->fold_chunk_size,
                            avg_fit/efti_config.ensemble_size,
                            accuracy,
                            leaves,
                            avg_size/efti_config.ensemble_size,
                            t_hb,
                            efti_config.ensemble_size
                    );
#endif
                /* return 0; */
                for (e = 0; e < efti_config.ensemble_size; e++)
                {
                    tree_delete_node(dt[e]);
                }
            }
        }
    }

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
        {"impurity_w",  optional_argument, 0,  'i' },
        {"impurity_topomut",  optional_argument, 0,  'a' },
        {"impurity_weightmut",  optional_argument, 0,  'b' },
        {"ensemble_size",  optional_argument, 0,  'e' },
        {"dataset_selection",  optional_argument, 0,  'd' },
        {0,           0,                 0,  0   }
    };

    int long_index =0;
    while ((opt = getopt_long(argc, argv,"m::t::w::s::x::y::z::r::o::i::a::b::d::",
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
        case 'i' : efti_config.impurity_weight = atof(optarg);
            break;
        case 'a' : efti_config.use_impurity_topo_mut = atof(optarg);
            break;
        case 'b' : efti_config.use_impurity_weight_mut = atof(optarg);
            break;
        case 'e' : efti_config.ensemble_size = atoi(optarg);
            break;
        case 'd' : efti_config.dataset_selection = optarg;
            efti_printf("Optarg: %s\n", optarg);
            break;
        default: print_usage();
            exit(EXIT_FAILURE);
        }
    }

    return crossvalidation();
}
