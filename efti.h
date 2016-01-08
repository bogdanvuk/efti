#ifndef __HEREBOY_H
#define __HEREBOY_H

#include <math.h>
#include <stdint.h>
#include "tree.h"

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
}Efti_Conf_t;

int efti_load_instance(int32_t* instance, uint_fast16_t category);
void efti_init();
void efti_reset(Efti_Conf_t *conf, int attribute_cnt, int maximum_category);
tree_node* efti(float* fitness, uint32_t* dt_leaves_cnt, uint32_t* dt_nonleaves_cnt, float* t_hb, float* t_fitness_calc_avg, float* tot_reclass);
void efti_close();
float efti_eval(tree_node* dt);

#endif
