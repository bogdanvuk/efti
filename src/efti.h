#ifndef __EFTI_H
#define __EFTI_H

#include <math.h>
#include <stdint.h>
#include "tree.h"
#include "dataset.h"

#define EFTI_PRINT_PROGRESS_INTERVAL	1000
#define EFTI_PRINT_STATS				1
#define DELTA_CLASSIFICATION    1
#define DELTA_ON_DEPTH_THR      3
#define DELTA_OFF_DEPTH_THR      1
#define EFTI_PRINT_DTS          0
#define CUSTOM_RAND 0

#define DT_USE_LOOP_UNFOLD			0
#define LEAVES_MAX				NUM_NODES

#define SEARCH_NONE 0
#define SEARCH_EFTI_METROPOLIS 1
#define SEARCH_HEREBOY 2
#define SEARCH_EFTI_LOG_METROPOLIS 3
/* #define SEARCH_EFTI_METROPOLIS 0 */

typedef struct {
    unsigned max_iterations;
    float topology_mutation_rate;
    float weights_mutation_rate;
    float search_probability;
    float search_probability_raise_due_to_stagnation_step;
    float topo_mutation_rate_raise_due_to_stagnation_step;
    float weight_mutation_rate_raise_due_to_stagnation_step;
    float return_to_best_prob_iteration_increment;
    float complexity_weight;
    float impurity_weight;
    int use_impurity_topo_mut;
    int use_impurity_weight_mut;
    int ensemble_size;
    int runs;
    int folds;
    char* dataset_selection;
    unsigned int seed;
    unsigned int search_function;
    unsigned int allow_subseq_searches;
    float max_time;
}Efti_Conf_t;


typedef struct {
    float fit;
    float oversize;
    float impurity;
    float accuracy;
    uint_fast16_t depth;
    uint_fast16_t leaves_cnt;
    uint_fast16_t nonleaves_cnt;
    tree_node* leaves[LEAVES_MAX];
    tree_node* nonleaves[LEAVES_MAX];
    tree_node* root;
}DT_t;

void dt_free(DT_t* dt);
int efti_load_instance(const int32_t* instance, uint_fast16_t category);
void efti_init();
// void efti_reset(const Efti_Conf_t *conf, T_Dataset* ds);
void efti_reset(const Efti_Conf_t *conf, int attr_cnt_, int categ_max_);
DT_t* efti(float* t_hb, uint_fast16_t* iters);
void efti_close();
float efti_eval(tree_node* dt);
float ensemble_eval(DT_t* dt[], int ensemble_size);

#endif
