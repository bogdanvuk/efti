
#    This file is part of DEAP.
#
#    DEAP is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Lesser General Public License as
#    published by the Free Software Foundation, either version 3 of
#    the License, or (at your option) any later version.
#
#    DEAP is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with DEAP. If not, see <http://www.gnu.org/licenses/>.

import array
import random
from time import time,sleep
import os
from cv_efti_single import run_tests, tests_all, param_def
from rank import load_data, mean_confidence_interval
from combine_res import merge_files

from deap import base
from deap import creator
from deap import benchmarks
from deap import tools

efti_config = [
    0.5,           # search_probability;
    0.1,           # search_probability_raise_due_to_stagnation_step;
    0.1,           # return_to_best_prob_iteration_increment;
 ]

efti_config_bound_low = [
    0,           # search_probability;
    0,           # search_probability_raise_due_to_stagnation_step;
    0.04,        # return_to_best_prob_iteration_increment;
 ]

efti_config_bound_high = [
    0.8,          # search_probability;
    0.5,          # search_probability_raise_due_to_stagnation_step;
    0.3,          # return_to_best_prob_iteration_increment;
 ]

def rescale(vector):
    res = []
    for v,l,u in zip(vector, efti_config_bound_low, efti_config_bound_high):
      res.append((v - l) / (u - l))

    return res

def descale(vector):
    res = []
    for v,l,u in zip(vector, efti_config_bound_low, efti_config_bound_high):
       res.append(l + v * (u - l))

    return res

def fitness_eval(fn, complexity_weight):
    res, cvs = load_data([fn], complexity_weight)

    fit = 0
    for ds in sorted(res['fit']):
        ds_fit = mean_confidence_interval(res['fit'][ds][0])[0]
        fit += ds_fit

    return fit

iter_cnt = 0
max_iter = 50000
max_iter_inc = 1
complexity_weight = 0.02

def evaluator(individual):
    global iter_cnt
    global max_iter

    individual = descale(individual)

    param_def['max_iter'] = max_iter
    param_def['oversize_w'] = complexity_weight
    param_def['topo_mut'] = individual[0]
    param_def['weight_mut'] = individual[1]
    param_def['search_prob'] = individual[2]
    param_def['s_accel_stagn'] = individual[3]
    param_def['t_accel_stagn'] = individual[4]
    param_def['w_accel_stagn'] = individual[5]
    param_def['return_prob'] = individual[6]

    # Hack! Wait for all threads to finish writing results to .js files
    sleep(10)

    params = run_tests(tests_all, threads=7, res_dir = './meta_results')
    thread_files = [t['log'] for t in params]

    fit_res_fn = 'iter_{}.js'.format(iter_cnt)
    merge_files(fit_res_fn, thread_files)

    fit = fitness_eval(fit_res_fn, complexity_weight)

    print("Iteration {}, max_iter: {}, fitness: {}".format(iter_cnt, max_iter, fit))
    print("Current individual: {}".format(individual))

    iter_cnt += 1
    max_iter = int(max_iter * max_iter_inc)

    return fit,

creator.create("FitnessMin", base.Fitness, weights=(-1.0,))
creator.create("Individual", array.array, typecode='d', fitness=creator.FitnessMin)

def update(ind, mu, std):
    for i, mu_i in enumerate(mu):
        ind[i] = random.gauss(mu_i,std)
        if ind[i] < 0:
            ind[i] = 0
        elif ind[i] > 1:
            ind[i] = 1;

toolbox = base.Toolbox()
toolbox.register("update", update)
toolbox.register("evaluate", evaluator)

def main():
    """Implements the One-Fifth rule algorithm as expressed in :
    Kern, S., S.D. Muller, N. Hansen, D. Buche, J. Ocenasek and P. Koumoutsakos (2004).
    Learning Probability Distributions in Continuous Evolutionary Algorithms -
    A Comparative Review. Natural Computing, 3(1), pp. 77-112.

    However instead of parent and offspring the algorithm is expressed in terms of
    best and worst. Best is equivalent to the parent, and worst to the offspring.
    Instead of producing a new individual each time, we have defined a function which
    updates the worst individual using the best one as the mean of the gaussian and
    the sigma computed as the standard deviation.
    """
    random.seed(64)

    logbook = tools.Logbook()
    logbook.header = "gen", "fitness"

    mu = rescale(efti_config)
    sigma = 0.05
    alpha = 2.0**(1.0/len(efti_config))

    best = creator.Individual(mu)
    best.fitness.values = toolbox.evaluate(best)
    worst = creator.Individual((0.0,)*len(efti_config))

    NGEN = 50
    for g in range(NGEN):
        toolbox.update(worst, best, sigma)
        worst.fitness.values = toolbox.evaluate(worst)
        if best.fitness >= worst.fitness:
            sigma = sigma * alpha
            best, worst = worst, best
        else:
            sigma = sigma * alpha**(-0.25)

        logbook.record(gen=g, fitness=best.fitness.values)
        print(logbook.stream)

    return best

if __name__ == "__main__":
    best = main()
    print(best)
