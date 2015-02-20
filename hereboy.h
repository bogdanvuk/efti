#ifndef __HEREBOY_H
#define __HEREBOY_H

#include <math.h>
#include <stdint.h>
#include "tree.h"

#define HB_HW						0
#define HB_HW_SW_FITNESS			0
#define HB_SW						1

#define HB_PRINT_PROGRESS_INTERVAL	100000
#define HB_PRINT_STATS				0

#define DT_USE_LOOP_UNFOLD			0

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
}Hb_Conf_t;

int hb_load_instance(int32_t* instance, uint_fast16_t category);
void hb_init();
void hb_reset(Hb_Conf_t *conf, int attribute_cnt, int maximum_category);
tree_node* hb_run(float* fitness, uint32_t* dt_leaves_cnt, uint32_t* dt_nonleaves_cnt, float* t_hb, float* t_fitness_calc_avg);
void hb_close();
float hb_eval(tree_node* dt);

#endif
