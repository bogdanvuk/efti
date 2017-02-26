/*
 * Empty C++ Application
 */

#include <stdlib.h>
#include <stdio.h>
#include "rand.h"
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
/* #include "crossvalid.h" */
#include <string>
#include <vector>
#include <H5Cpp.h>
#include <iostream>
#include <vector>

using namespace H5;

#define ENSEMBLE_SIZE_MAX         32
#define MAX_ITERATIONS            50000
#define SEED                      29

Efti_Conf_t efti_config = {
    MAX_ITERATIONS,	// max_iterations
    0.6,          // topology_mutation_rate;
    0.0,           // weights_mutation_rate;
    0.05,           // search_probability;
    5e-5,          // search_probability_raise_due_to_stagnation_step;
    1,        // topo_mutation_rate_raise_due_to_stagnation_step;
    0.0,        // weight_mutation_rate_raise_due_to_stagnation_step;
    1e-4,           // return_to_best_prob_iteration_increment;
    0.01,            // complexity_weight;
    0,              // impurity_weight;
    0,              // use_impurity_topo_mut;
    0,              // use_impurity_weight_mut;
    1,              // ensemble_size
    1,              // runs
    1,              // folds
    NULL,           // dataset_list
    SEED,           // seed
    SEARCH_EFTI_METROPOLIS, // search_function
    1,               // allow_subseq_searches
    -1              // max_time
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

int file_load_dataset_to_efti(const Efti_Conf_t *conf){
	std::string ifn = "/data/projects/dtnn/deep-learning-models/features.h5";

    // Open HDF5 file handle, read only
    H5File fp(ifn.c_str(),H5F_ACC_RDONLY);

	int categ_max = fp.getNumObjs();

	efti_reset(conf, 25088, categ_max);

	std::cout << "Number of categories: " << categ_max << std::endl;

	for(int c=0; c < categ_max; c++) {
		Group grp = fp.openGroup(fp.getObjnameByIdx(c));

		int categ_inst = grp.getNumObjs();

		for(int i=0; i< categ_inst; i++) {
			// access the required dataset by path name
			DataSet dset = grp.openDataSet(grp.getObjnameByIdx(i));

			// get the dataspace
			DataSpace dspace = dset.getSpace();

			// get the dataset type class
			H5T_class_t type_class = dset.getTypeClass();
			// According to HDFView, this is a 32-bit floating-point

			// get the size of the dataset
			hsize_t rank;
			hsize_t dims[2];
			rank = dspace.getSimpleExtentDims(dims, NULL); // rank = 1

			// Define the memory dataspace
			hsize_t dimsm[1];
			dimsm[0] = dims[0];
			DataSpace memspace (1,dimsm);

			std::vector<int32_t> data;
			data.resize(dims[0]);

			/* int16_t data[25088]; */

			dset.read(data.data(), PredType::NATIVE_INT, memspace, dspace); // FAILS

			efti_load_instance(data.data(), c+1);

		}
	}

	fp.close();
}

int crossvalidation()
{
    int i, k, n, e;
    DT_t* dt[ENSEMBLE_SIZE_MAX];
    float avg_fit;
    float avg_size;
    float accuracy;
    float t_hb;
    uint_fast16_t iters;
    int train_num;
    /* Cv_Status_T* cv_conf; */

//	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
//					(Xil_ExceptionHandler)MyDataAbortHandler,
//					NULL);


    efti_init();
    rand_init(efti_config.seed);

    efti_printf("$efti_config:max_iterations=%d,topology_mutation_rate=%e,"
                "topo_mutation_rate_raise_due_to_stagnation_step=%e,"
                "weights_mutation_rate=%e,search_probability=%e,"
                "search_probability_raise_due_to_stagnation_step=%e,"
                "weight_mutation_rate_raise_due_to_stagnation_step=%e,"
                "return_to_best_prob_iteration_increment=%e,"
                "complexity_weight=%e,impurity_weight=%e,use_impurity_topo_mut=%d,"
                "use_impurity_weight_mut=%d,ensemble_size=%d,seed=%d,"
                "search_function=%d,allow_subseq_searches=%d,"
                "max_time=%f\n",
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
                efti_config.seed,
                efti_config.search_function,
                efti_config.allow_subseq_searches,
                efti_config.max_time
        );

    /* cv_conf = crossvalid_init(efti_config.dataset_selection, 1, efti_config.seed, efti_config.folds, efti_config.runs); */

    /* for (i = 0; i < cv_conf->datasets_num; i++) */
    /* { */
    /*     for (n=0; n < efti_config.runs; n++) */
    /*     { */

    /*         for (k = 0; k < efti_config.folds; k++) */
    /*         { */
    /*             avg_fit = 0; */
    /*             avg_size = 0; */
    /*             cv_conf = crossvalid_next_conf(); */
                for (e = 0; e < efti_config.ensemble_size; e++)
                {
                    /* efti_reset(&efti_config, cv_conf->dataset); */
                    /* train_num = load_dataset_to_efti(cv_conf->dataset, cv_conf->perm, */
                    /*                                  cv_conf->chunk_start, cv_conf->chunk_end, */
                    /*                                  cv_conf->fold_start, cv_conf->fold_start + cv_conf->fold_chunk_size); */
					file_load_dataset_to_efti(&efti_config);
                    dt[e] = efti(&t_hb, &iters);
                    avg_fit += dt[e]->fit;
                    avg_size += dt[e]->nonleaves_cnt;
                }

/* #if EFTI_PROFILING == 0 */

/*                 if (cv_conf->fold_chunk_size > 0) { */
/*                     load_dataset_to_efti(cv_conf->dataset, cv_conf->perm, */
/*                                          cv_conf->fold_start, */
/*                                          cv_conf->fold_start + cv_conf->fold_chunk_size, */
/*                                          0, 0); */

/*                     accuracy =  ensemble_eval(dt, efti_config.ensemble_size); */
/*                     efti_printf("$cv_pc_run:dataset=\"%s\",run=%d,id=%d,train_range=(%d,%d)," */
/*                                 "train_cnt=%d,test_range=(%d,%d),test_cnt=%d,fitness=%f,accuracy=%f," */
/*                                 "leaves=%d,depth=%d,nonleaves=%f,time=%f,ensemble_size=%d," */
/*                                 "iters=%d\n", */
/*                                 cv_conf->dataset->name, */
/*                                 n, */
/*                                 k, */
/*                                 cv_conf->chunk_start, */
/*                                 cv_conf->chunk_end, */
/*                                 cv_conf->chunk_size, */
/*                                 cv_conf->fold_start, */
/*                                 cv_conf->fold_start + cv_conf->fold_chunk_size, */
/*                                 cv_conf->fold_chunk_size, */
/*                                 avg_fit/efti_config.ensemble_size, */
/*                                 accuracy, */
/*                                 dt[0]->leaves_cnt, */
/*                                 dt[0]->depth, */
/*                                 avg_size/efti_config.ensemble_size, */
/*                                 t_hb, */
/*                                 efti_config.ensemble_size, */
/*                                 iters */
/*                         ); */
/*                 } */
/* #endif */
				std::cout << "Time elapsed: " << t_hb << std::endl;
                for (e = 0; e < efti_config.ensemble_size; e++)
                {
                    dt_free(dt[e]);
                    free(dt[e]);
                }

                // return 0;
            /* } */
    /*     } */
    /* } */

    efti_close();

    efti_printf("EXITING!\n");
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
        {"runs",  optional_argument, 0,  'n' },
        {"folds",  optional_argument, 0,  'f' },
        {"seed",  optional_argument, 0,  'c' },
        {"search_function",  optional_argument, 0,  'g' },
        {"allow_subseq_searches",  optional_argument, 0,  'h' },
        {"max_time",  optional_argument, 0,  'j' },
        {0,           0,                 0,  0   }
    };

    int long_index =0;
    while ((opt = getopt_long(argc, argv,"a::b::c::d::e::f::g::h::i::j::o::m::n::r::s::t::w::x::y::z::",
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
        case 'n' : efti_config.runs = atoi(optarg);
            break;
        case 'f' : efti_config.folds = atoi(optarg);
            break;
        case 'c' : efti_config.seed = atol(optarg);
            break;
        case 'g' : efti_config.search_function = atoi(optarg);
            break;
        case 'h' : efti_config.allow_subseq_searches = atoi(optarg);
            break;
        case 'j' : efti_config.max_time = atof(optarg);
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
