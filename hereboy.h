#ifndef __HEREBOY_H
#define __HEREBOY_H

#include <math.h>
#include <stdint.h>

#define HB_PRINT_PROGRESS_INTERVAL	50000
#define HB_PRINT_STATS				0
#define HB_PRINT_FINAL				1

typedef struct {
	unsigned max_iterations;
	double topology_mutation_rate;
	double weights_mutation_rate;
	double search_probability;
	double search_probability_raise_due_to_stagnation_step;
	double topo_mutation_rate_raise_due_to_stagnation_step;
	double weight_mutation_rate_raise_due_to_stagnation_step;
	double return_to_best_prob_iteration_increment;
	double complexity_weight;
}Hb_Conf_t;

int hb_load_instance(int16_t* instance, uint_fast16_t category);
int hb_init(Hb_Conf_t *conf, int attribute_cnt, int maximum_category);
int hb_run();

#endif
