#ifndef __HEREBOY_H
#define __HEREBOY_H

#include <math.h>
#include <stdint.h>
#include "tree.h"
#include "dataset.h"

#define EFTI_PRINT_PROGRESS_INTERVAL	100000
#define EFTI_PRINT_STATS				0

#define DT_USE_LOOP_UNFOLD			1

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
}Efti_Conf_t;

int efti_load_instance(const int32_t* instance, uint_fast16_t category);
void efti_init();
void efti_reset(const Efti_Conf_t *conf, T_Dataset* ds);
tree_node* efti(float* fitness, uint32_t* dt_leaves_cnt, uint32_t* dt_nonleaves_cnt, float* t_hb, unsigned int *seed);
void efti_close();
float efti_eval(tree_node* dt);
float ensemble_eval(tree_node* dt[], int ensemble_size);

#endif
