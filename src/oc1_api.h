
#ifndef OC1_API_HPP
#define OC1_API_HPP

#include <stdint.h>
#include "hw_config.h"

typedef struct point
{
	float *dimension;
	int category;
	double val; /*Value obtained by substituting this point in the 
				  equation of the hyperplane under consideration.
				  This field is maintained to avoid redundant
				  computation. */
}POINT;

float oblique_split(POINT **cur_points,int cur_no_of_points,char *cur_label);
void oc1_load_instance(const TAttr* instance, int category);
void init_oc1(int inst_cnt, int attr_cnt, int categ_max);
void oc1_split(TAttr* weights, TAttr* thr);
void reset_oc1();


#endif
