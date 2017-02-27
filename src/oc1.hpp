#ifndef OC1_HPP
#define OC1_HPP
/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : oc1.h                                            */
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : Data structure and constant definitions   */
/* Is used by modules in : All *.c files, except util.c.        */
/****************************************************************/
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "oc1_api.h"

/* Change the following statement to use a different impurity measure. */

//#define IMPURITY info_gain()
#define IMPURITY twoing()
/* possible values are: maxminority                             */
/*			summinority				*/
/*			variance				*/
/*			info_gain				*/
/*			gini_index				*/
/*			twoing					*/


#define NO_OF_STD_ERRORS	0 /* used for cost complexity pruning, 
                                     in prune.c */
#define SEQUENTIAL 		0
#define BEST_FIRST 	 	1	
#define RANDOM                  2

#define CORRECT 		1
#define INCORRECT 		0

#define TRUE 			1
#define FALSE 			0

#define LEFT 			0
#define RIGHT 			1

#define LESS_THAN 		0
#define MORE_THAN 		1

#define MAX_COEFFICIENT 	1.0
#define MAX_NO_OF_ATTRIBUTES	4020
#define MAX_DT_DEPTH 		50 
#define MAX_NO_OF_STAGNANT_PERTURBATIONS 	10
#define MAX_CART_CYCLES         100

#define TOLERANCE		0.0001
#define TOO_SMALL_THRESHOLD	2.0
#define TOO_SMALL_FOR_ANY_SPLIT		3
#define TOO_SMALL_FOR_OBLIQUE_SPLIT	2 * no_of_dimensions

#define TRAIN			1
#define	TEST			2

#define LINESIZE 		80000
#define MISSING_VALUE           -1.0 * HUGE_VAL

#define translatex(x) ((x - xmin) * (pmaxx - pminx) / (xmax - xmin) + pminx)
#define translatey(y) ((y - ymin) * (pmaxy - pminy) / (ymax - ymin) + pminy)




struct endpoint
 {
  float x,y;
 };

typedef struct edge
 {
  struct endpoint from,to;
 }EDGE;

struct tree_node
 {
  float *coefficients;
  int *left_count, *right_count;
  struct tree_node *parent,*left,*right;
  int left_cat,right_cat;
  char label[MAX_DT_DEPTH];
  float alpha; /* used only in error_complexity pruning. */
  int no_of_points;
  EDGE edge; /* used only in the display module. */
 };

struct unidim
 {
  float value;
  int cat;
 };

void free_vector(float *v,int nl,int nh);
float *vector(int nl,int nh);
int largest_element(int *array,int count);
void error(const char* error_text);
void reset_counts();
void find_values(POINT **cur_points, int cur_no_of_points);
void generate_random_hyperplane(float* array_name,int length, float max_value);

void set_counts(POINT** cur_points,int cur_no_of_points,int flag);
float compute_impurity(int cur_no_of_points);
float perturb_randomly(POINT **cur_points, int cur_no_of_points, float cur_error, char* cur_label);
float myrandom(float above,float below);
float myabs(float x);
float linear_split(int no_of_eff_points);
float twoing();
int *ivector(int nl,int nh);
void free_ivector(int *v,int nl,int nh);
float mylog2(double x);
double *dvector(int nl, int nh);
float cart_perturb(POINT **cur_points, int cur_no_of_points, int cur_coeff, float cur_error);
float cart_perturb_constant(POINT **cur_points, int cur_no_of_points, float cur_error);

extern int no_of_dimensions;
extern int *left_count,*right_count;
extern int no_of_categories;
extern int coeff_modified;
extern float *coeff_array;
extern int no_of_coeffs;
extern int no_of_dimensions,no_of_categories;
extern int *left_count,*right_count;
extern int coeff_modified;
extern float *coeff_array;
extern float *modified_coeff_array;
extern struct unidim *candidates;
extern double *temp_val; /*Work area */
extern int veryverbose;


#endif
