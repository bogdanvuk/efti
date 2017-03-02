#include "oc1.hpp"

/* #include <iostream> */

float *coeff_array,*modified_coeff_array,*best_coeff_array;
int no_of_dimensions=0,no_of_coeffs;
int no_of_categories=0;
int coeff_modified = FALSE;
int no_of_restarts=4,no_of_folds=0;
int unlabeled = FALSE,verbose=FALSE,veryverbose = FALSE;
int order_of_perturbation = SEQUENTIAL;
int no_of_stagnant_perturbations,no_of_missing_values=0;
int cycle_count=0;
int max_no_of_random_perturbations = 5;
int *left_count=NULL, *right_count=NULL;
struct unidim *candidates;
double *temp_val;
POINT **train_points=NULL,**test_points=NULL;
int no_of_train_points=0,no_of_test_points=0;

int alter_coefficients(POINT **cur_points, int cur_no_of_points);
float cart_perturb(POINT **cur_points, int cur_no_of_points, int cur_coeff, float cur_error);
float suggest_perturbation(POINT **cur_points, int cur_no_of_points, int cur_coeff, float cur_error);
POINT **allocate_point_array(POINT **array_name, int size, int prev_size);
void allocate_structures(int no_of_points);
float cart_split(POINT **cur_points,int cur_no_of_points,char* cur_label);
float axis_parallel_split(POINT **cur_points, int cur_no_of_points);

void init_oc1(int inst_cnt, int attr_cnt, int categ_max) {
	no_of_train_points = 0;
	no_of_dimensions = attr_cnt;
	no_of_categories = categ_max;
	allocate_structures(inst_cnt);
	train_points = allocate_point_array(train_points,inst_cnt+1,0);
}

void reset_oc1() {
	no_of_train_points = 0;
}

void oc1_load_instance(const TAttr* instance, int category) {
	no_of_train_points++;

    for (int i = 1; i < no_of_dimensions; i++)
    {
		/* train_points[no_of_train_points]->dimension[i] = ((float)(instance[i-1] + 1))/32767; */
		train_points[no_of_train_points]->dimension[i] = instance[i-1];
    }
	train_points[no_of_train_points]->category = category;
	train_points[no_of_train_points]->val = 0.0;

}

void oc1_split(TAttr* weights, TAttr* thr) {
	oblique_split(train_points,no_of_train_points,NULL);

	/* for (int i = 1; i <= no_of_dimensions; i++) { */
	/* 	weights[i-1] = (int32_t)(coeff_array[i] * 32767); */
	/* } */
	/* *thr = (int32_t)(coeff_array[no_of_coeffs] * 32767); */
	for (int i = 1; i <= no_of_dimensions; i++) {
		weights[i-1] = coeff_array[i];
	}
	// OC1 operates with implicit test equation
	*thr = -coeff_array[no_of_coeffs];

	/* float cur_impurity; */
	/* cur_impurity = axis_parallel_split(train_points,no_of_train_points); */
	/* /\* if (cur_impurity && (strlen(node_str) == 0 || *\/ */
	/* /\* 					 cur_no_of_points > TOO_SMALL_FOR_OBLIQUE_SPLIT)) *\/ */
	/* cur_impurity = cart_split(train_points,no_of_train_points, NULL); */
}

/************************************************************************/
/* Module name : oblique_split						*/
/* Functionality : 	Attempts to find the hyperplane, at an unrestri-*/
/*			cted orientation, that best separates 		*/
/*			"cur_points" (minimizing the current impurity	*/
/*			measure), using hill climbing and randomization.*/
/* Parameters :	cur_points : array of pointers to the points (samples)	*/
/*			     under consideration.			*/
/*		cur_no_of_points : number of points under consideration.*/
/* Returns :	the impurity measure of the best hyperplane found.	*/
/*		The hyperplane itself is returned through the global	*/
/*		array "coeff_array".					*/
/* Calls modules :	generate_random_hyperplane			*/
/*			find_values (perturb.c)				*/
/*			set_counts (compute_impurity.c)			*/
/*			comp
/*			myrandom (util.c)				*/
/*			suggest_perturbation (perturb.c)		*/
/*			perturb_randomly (perturb.c)			*/
/* Is called by modules :	build_subtree				*/
/************************************************************************/
float oblique_split(POINT **cur_points,int cur_no_of_points,char *cur_label)
{
    char c;
    int i,j,old_nsp,restart_count = 1;
    int cur_coeff,improved_in_this_cycle,best_coeff_to_improve;
    float cur_error,old_cur_error,best_cur_error,least_error;
    float x,changeinval;
    float new_error;


	generate_random_hyperplane(coeff_array,no_of_coeffs,MAX_COEFFICIENT);
	coeff_modified = TRUE;

    find_values(cur_points,cur_no_of_points);
    set_counts(cur_points,cur_no_of_points,1);
    least_error = cur_error = compute_impurity(cur_no_of_points);
    for (i=1;i<=no_of_coeffs;i++) best_coeff_array[i] = coeff_array[i];
    // write_hyperplane(animationfile,cur_label);
  
    /* Repeat this loop once for every restart*/
    while (least_error != 0.0 && restart_count <= no_of_restarts)
    {
        if (veryverbose)
            printf(" Restart %d: Initial Impurity = %.3f\n",restart_count,cur_error);
      
        no_of_stagnant_perturbations = 0;
        if (order_of_perturbation == RANDOM)
        {
            // if (cycle_count <= 0) cycle_count = 10 * no_of_coeffs;
            // for (i=1;i<=cycle_count;i++)
            // {
            //     if (cur_error == 0.0) break;
            //     cur_coeff = 0;
            //     while (!cur_coeff)
            //         cur_coeff = (int)myrandom(1.0,(float)(no_of_coeffs+1));
	      
            //     new_error = suggest_perturbation(cur_points,cur_no_of_points,
            //                                      cur_coeff,cur_error);
            //     if (new_error <= cur_error &&
            //         alter_coefficients(cur_points,cur_no_of_points))
            //     {
            //         if (veryverbose)
            //             printf("\thill climbing for coeff. %d. impurity %.3f -> %.3f\n",
            //                    cur_coeff,cur_error,new_error);
            //         cur_error = new_error;
            //         improved_in_this_cycle = TRUE;
            //         // write_hyperplane(animationfile,cur_label);
            //         if (cur_error == 0) break;
            //     }
            //     else /*Try improving in a random direction*/
            //     {
            //         improved_in_this_cycle = FALSE;
            //         j = 0;
            //         while (cur_error != 0 &&
            //                !improved_in_this_cycle &&
            //                ++j<=max_no_of_random_perturbations)
            //         {
            //             new_error = perturb_randomly
            //                 (cur_points,cur_no_of_points,cur_error);
            //             if (alter_coefficients(cur_points,cur_no_of_points))
            //             {
            //                 if (veryverbose)
            //                     printf("\trandom jump. impurity %.3f -> %.3f\n",
            //                            cur_error,new_error);
            //                 cur_error = new_error;
            //                 improved_in_this_cycle = TRUE;
            //                 write_hyperplane(animationfile,cur_label);
            //             }
            //         }
            //     }
            // }
        }
      
        else /* best_first or sequential orders of perturbation.*/
        {
            improved_in_this_cycle = TRUE;
            cycle_count = 0;
	  
            while (improved_in_this_cycle)
            {
                if (cur_error == 0.0) break;
                cycle_count++;
                improved_in_this_cycle = FALSE;
	      
                if (order_of_perturbation == BEST_FIRST)
                {
                    best_cur_error = HUGE_VAL;
                    best_coeff_to_improve = 1;
                    old_nsp = no_of_stagnant_perturbations;
                }
	      
                for (cur_coeff = 1; cur_coeff < no_of_coeffs;cur_coeff++)
                {
                    new_error = suggest_perturbation(cur_points,cur_no_of_points,
                                                     cur_coeff,cur_error);
                    if (order_of_perturbation == BEST_FIRST)
                    {
                        if (new_error < best_cur_error)
                        {
                            best_cur_error = new_error;
                            best_coeff_to_improve = cur_coeff;
                        }
                        no_of_stagnant_perturbations = old_nsp;
                        if (best_cur_error == 0) break;
                    }
                    else if (new_error <= cur_error &&
                             alter_coefficients(cur_points,cur_no_of_points))
                    {
                        if (veryverbose)
                            printf("\thill climbing for coeff. %d. impurity %.3f -> %.3f\n",
                                   cur_coeff,cur_error,new_error);
                        cur_error = new_error;
                        improved_in_this_cycle = TRUE;
                        // write_hyperplane(animationfile,cur_label);
                        if (cur_error == 0) break;
                    }
                }
	      
                if (order_of_perturbation == BEST_FIRST
                    && best_cur_error <= cur_error)
                {
                    cur_coeff = best_coeff_to_improve;
                    new_error = suggest_perturbation(cur_points,cur_no_of_points,
                                                     cur_coeff,cur_error);
                    if (alter_coefficients(cur_points,cur_no_of_points))
                    {
                        if (veryverbose)
                            printf("\thill climbing for coeff. %d. impurity %.3f -> %.3f\n",
                                   cur_coeff,cur_error,new_error);
                        cur_error = new_error;
                        improved_in_this_cycle = TRUE;
                        // write_hyperplane(animationfile,cur_label);
                    }
                }
	      
                if (cur_error != 0 && !improved_in_this_cycle)
                    /*Try improving along a random direction*/
                {
                    i = 0;
                    while (cur_error != 0 &&
                           !improved_in_this_cycle &&
                           ++i<=max_no_of_random_perturbations)
                    {
                        new_error = perturb_randomly(cur_points,cur_no_of_points,
                                                     cur_error,cur_label);
                        if (alter_coefficients(cur_points,cur_no_of_points))
                        {
                            if (veryverbose)
                                printf("\trandom jump. impurity %.3f -> %.3f\n",
                                       cur_error,new_error);
                            cur_error = new_error;
                            improved_in_this_cycle = TRUE;
                            // write_hyperplane(animationfile,cur_label);
                        }
                    }
                }
            }
        }
 
        if (cur_error < least_error ||
            (cur_error == least_error && myrandom(0.0,1.0) > 0.5))
        {
            least_error = cur_error;
            for (i=1;i<=no_of_coeffs;i++) best_coeff_array[i] = coeff_array[i];
        }
      
        if (least_error != 0 && ++restart_count <= no_of_restarts)
        {
            generate_random_hyperplane(coeff_array,no_of_coeffs,MAX_COEFFICIENT);
            coeff_modified = TRUE;
            find_values(cur_points,cur_no_of_points);
            set_counts(cur_points,cur_no_of_points,1);
            cur_error = compute_impurity(cur_no_of_points);
            // write_hyperplane(animationfile,cur_label);
        }
    }
  
    for (i=1;i<=no_of_coeffs;i++)
        coeff_array[i] = best_coeff_array[i];
    coeff_modified = TRUE;
    find_values(cur_points,cur_no_of_points);
    set_counts(cur_points,cur_no_of_points,1);
    return(least_error);

}

/************************************************************************/
/* Module name : Alter_Coefficients                                     */
/* Functionality : First checks if any coefficient values are changed   */
/*                 considerably (> TOLERANCE) in the last perturbation. */
/*                 If they are, updates the "val" fields of the points  */
/*                 to correspond to the new hyperplane. Sets the left_  */
/*                 count and right_count arrays.                        */
/* Parameters : cur_points : Array of pointers to the points            */
/*              cur_no_of_points.                                       */
/* Returns : 1  if any coefficient values are altered,                  */
/*           0  otherwise                                               */
/* Calls modules : myabs (util.c)                                       */
/*                 set_counts (compute_impurity.c)                      */
/* Is called by modules : oblique_split                                 */
/* Remarks : Assumes that the arrays coeff_array, modified_coeff_array  */
/*           are set. Assumes that the "val" fields of the points       */
/*           correspond to the coefficient values in coeff_array.       */  
/************************************************************************/
int alter_coefficients(POINT **cur_points, int cur_no_of_points)
{
    int i,j=0;
  
    for (i=1;i<=no_of_coeffs;i++)
        if (myabs(coeff_array[i]-modified_coeff_array[i]) > TOLERANCE)
        {
            if (i != no_of_coeffs)
                for (j=1;j<=cur_no_of_points;j++)
                    cur_points[j]->val += (modified_coeff_array[i] - coeff_array[i]) *
                        cur_points[j]->dimension[i];
            else
                for (j=1;j<=cur_no_of_points;j++)
                    cur_points[j]->val += (modified_coeff_array[i] - coeff_array[i]);
	
            coeff_array[i] = modified_coeff_array[i];
        }
    if (j != 0)
    {
        set_counts(cur_points,cur_no_of_points,1);
        return(1);
    }
    else return(0);
}

POINT **allocate_point_array(POINT **array_name, int size, int prev_size)
{
  int i;
  
  if (prev_size == 0)
    {
      if (array_name != NULL) 
	free((char *)(array_name+1)); 
      
      array_name = (struct point **)malloc
	((unsigned)size * sizeof(struct point *)); 
      if (!array_name)
	error("Allocate_Point_Array: Memory Allocation Failure 1.");
      
      array_name -= 1; /* All indices start from 1*/
      
      for (i=1;i<=size;i++)
	{
	  array_name[i] = (struct point *)malloc((unsigned) sizeof(struct point)); 
	  if (!array_name[i])
	    error("Allocate_Point_Array : Memory Allocation failure 2.");
	}
      
      for (i=1;i<=size;i++)
	array_name[i]->dimension = vector(1,no_of_dimensions);
    }
  else
    {
      array_name += 1;
      array_name = (struct point **)realloc
	(array_name, (unsigned)size * sizeof(struct point *)); 
      if (!array_name)
	error("Allocate_Point_Array: Memory Allocation Failure 3.");
      
      array_name -= 1; /* All indices start from 1*/
      
      if (prev_size >= size) return(array_name);
      
      for (i=prev_size+1;i<=size;i++)
	{
	  array_name[i] = (struct point *)malloc((unsigned) sizeof(struct point)); 
	  if (!array_name[i])
	    error("Allocate_Point_Array : Memory Allocation failure 4.");
	}
      
      for (i=prev_size+1;i<=size;i++)
	array_name[i]->dimension = vector(1,no_of_dimensions);
    }
  
  return(array_name);
}

void allocate_structures(int no_of_points)
{
    int i;
  
    no_of_coeffs = no_of_dimensions+1;
    coeff_array = vector(1,no_of_coeffs);
    modified_coeff_array = vector(1,no_of_coeffs);
    best_coeff_array = vector(1,no_of_coeffs);
    left_count = ivector(1,no_of_categories);
    right_count = ivector(1,no_of_categories);
    candidates = (struct unidim *)malloc((unsigned)no_of_points *
                                         sizeof(struct unidim));
    candidates -= 1;
    /* attribute_min = vector(1,no_of_dimensions); */
    /* attribute_avg = vector(1,no_of_dimensions); */
    /* attribute_sdev = vector(1,no_of_dimensions); */
    temp_val = dvector(1,no_of_points);
}

/************************************************************************/
/* Module name : 	axis_parallel_split				*/
/* Functionality : 	Attempts to find the hyperplane, at an axis-	*/
/*			parallel orientation, that best separates	*/
/*			"cur_points" (minimizing the current impurity	*/
/*			measure). 					*/
/* Parameters :	cur_points : array of pointers to the points (samples)	*/
/*			     under consideration.			*/
/*		cur_no_of_points : number of points under consideration.*/
/* Returns :	the impurity of the best hyperplane found.	        */
/*		The hyperplane itself is returned through the global	*/
/*		array "coeff_array".					*/
/* Calls modules :	linear_split (perturb.c)			*/
/*			find_values (perturb.c)				*/
/*			set_counts (compute_impurity.c)			*/
/*			compute_impurity (compute_impurity.c)		*/
/* Is called by modules :	build_subtree				*/
/************************************************************************/
float axis_parallel_split(POINT **cur_points, int cur_no_of_points)
{
    int i,j,cur_coeff,best_coeff;
    float cur_error,best_error,best_coeff_split_at;
  
    for (i=1;i<=no_of_coeffs;i++) coeff_array[i] = 0;
  
    for (cur_coeff=1;cur_coeff<=no_of_dimensions;cur_coeff++)
    {
		/* printf(" Processing coefficient  %d",cur_coeff); */
		if ((cur_coeff % 10) == 0)
            printf(" Processing coefficient  %d\n",cur_coeff);

        coeff_array[cur_coeff] = 1;
        for (j=1;j<=cur_no_of_points;j++)
        {
            candidates[j].value = cur_points[j]->dimension[cur_coeff];
            candidates[j].cat   = cur_points[j]->category;
        }
        coeff_array[no_of_coeffs] =
            -1.0 * (float)linear_split(cur_no_of_points);
      
        coeff_modified = TRUE;
        find_values(cur_points,cur_no_of_points);
        set_counts(cur_points,cur_no_of_points,1);
        cur_error = compute_impurity(cur_no_of_points);
      
        if  (cur_coeff == 1 || cur_error < best_error)
        {
            best_coeff = cur_coeff;
            best_coeff_split_at = coeff_array[no_of_coeffs];
            best_error = cur_error;
        }
      
        coeff_array[cur_coeff] = 0;
        coeff_array[no_of_coeffs] = 0;
      
        if (best_error == 0) break;
    }
  
  
    coeff_array[best_coeff] = 1;
    coeff_array[no_of_coeffs] = best_coeff_split_at;
    coeff_modified = TRUE;
  
    return(best_error);
}

/************************************************************************/
/* Module name : Cart_Split                                             */
/* Functionality : Implements the CART-Linear Combinations (Breiman et  */
/*                 al, 1984, Chapter 5) hill climbing coefficient       */
/*                 perturbation algorithm.                              */
/* Parameters : cur_points: Array of pointers to the current points.    */
/*              cur_no_of_points:                                       */
/*              cur_label: Label of the tree node for which current     */
/*                         split is being induced.                      */
/* Returns : impurity of the induced hyperplane.                        */
/* Calls modules : cart_perturb (perturb.c)                             */
/*                 cart_perturb_constant (perturb.c)                    */
/*                 write_hyperplane                                     */
/*                 find_values (compute_impurity.c)                     */
/*                 set_counts (compute_impurity.c)                      */
/* Is called by modules : build_subtree                                 */
/* Remarks : See the CART book for a description of the algorithm.      */
/************************************************************************/
float cart_split(POINT **cur_points,int cur_no_of_points,char* cur_label)
{
    int cur_coeff;
    float cur_error,new_error,prev_impurity;

    /*Starts with the best axis parallel hyperplane. */
    /* write_hyperplane(animationfile,cur_label); */
    find_values(cur_points,cur_no_of_points);
    set_counts(cur_points,cur_no_of_points,1);
    cur_error = compute_impurity(cur_no_of_points);
    cycle_count = 0;

    while (TRUE)
    {
        if (cur_error == 0.0) break;
        cycle_count++;
        if (cycle_count != 1) prev_impurity = cur_error;
      
        for (cur_coeff = 1; cur_coeff < no_of_coeffs;cur_coeff++)
        {
            new_error = cart_perturb(cur_points,cur_no_of_points,cur_coeff,cur_error);
            if (alter_coefficients(cur_points,cur_no_of_points))
            {
                if (veryverbose)
                    printf("\tCART hill climbing for coeff. %d. impurity %.3f -> %.3f\n",
                           cur_coeff,cur_error,new_error);
                cur_error = new_error;
                /* write_hyperplane(animationfile,cur_label); */
                if (cur_error == 0) break;
            }
        }
        if (cur_error != 0)
        {
            new_error = cart_perturb_constant(cur_points,cur_no_of_points,cur_error);
            if (alter_coefficients(cur_points,cur_no_of_points))
            {
                if (veryverbose)
                    printf("\tCART hill climbing for coeff. %d. impurity %.3f -> %.3f\n",
                           no_of_coeffs,cur_error,new_error);
                cur_error = new_error;
                /* write_hyperplane(animationfile,cur_label); */
            }
        }
        if (cycle_count > MAX_CART_CYCLES)
            /* Cart multivariate algorithm can get stuck in some domains.
               Arbitrary tie breaker. */
            break;
      
        if (cycle_count != 1 && myabs(prev_impurity - cur_error)<TOLERANCE)
            break;
    }
  
    return(cur_error);
  
}

