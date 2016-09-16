#!/usr/bin/env python3
from random import Random
import os
from time import time,sleep
import inspyred
from cv_efti_single import run_tests, tests_all, param_def
from rank import load_data, mean_confidence_interval
from combine_res import merge_files
# import logging

#efti_config = [
#    0.002,          # topology_mutation_rate;
#    0.01,           # weights_mutation_rate;
#    0.01,           # search_probability;
#    0.001,          # search_probability_raise_due_to_stagnation_step;
#    0.00005,        # topo_mutation_rate_raise_due_to_stagnation_step;
#    0.00004,        # weight_mutation_rate_raise_due_to_stagnation_step;
#    5e-7,           # return_to_best_prob_iteration_increment;
# ]

efti_config = [
    0.433,          # topology_mutation_rate;
    0.01,           # weights_mutation_rate;
    0.01,           # search_probability;
    0.0,          # search_probability_raise_due_to_stagnation_step;
    0.00005,        # topo_mutation_rate_raise_due_to_stagnation_step;
    0.00004,        # weight_mutation_rate_raise_due_to_stagnation_step;
    5e-7,           # return_to_best_prob_iteration_increment;
 ]

iter_cnt = 0
max_iter = 5000
max_iter_inc = 1.08
complexity_weight = 0.02

def generator(random, args):
    return efti_config

def evaluator(candidates, args):
    global iter_cnt
    global max_iter

    param_def['max_iter'] = max_iter
    param_def['oversize_w'] = complexity_weight
    param_def['topo_mut'] = candidates[0][0]
    param_def['weight_mut'] = candidates[0][1]
    param_def['search_prob'] = candidates[0][2]
    param_def['s_accel_stagn'] = candidates[0][3]
    param_def['t_accel_stagn'] = candidates[0][4]
    param_def['w_accel_stagn'] = candidates[0][5]
    param_def['return_prob'] = candidates[0][6]

    # Hack! Wait for all threads to finish writing results to .js files
    sleep(10)

    params = run_tests(tests_all, threads=7, res_dir = './meta_results')
    thread_files = [t['log'] for t in params]

    fit_res_fn = 'iter_{}.js'.format(iter_cnt)
    merge_files(fit_res_fn, thread_files)

    res, cvs = load_data([fit_res_fn], complexity_weight)

    fit = 0
    for ds in res['fit']:
        fit += mean_confidence_interval(res['fit'][ds][0])[0]

    print("Iteration {}, max_iter: {}, fitness: {}".format(iter_cnt, max_iter, fit))

    iter_cnt += 1
    max_iter = int(max_iter * max_iter_inc)

    return [fit]

bounder = inspyred.ec.Bounder([0] * len(efti_config), [1,1,0.5,0.1,0.1,0.1,0.1])

def main(prng=None, display=False):
    os.makedirs('meta_results', exist_ok=True)
    iter_cnt = 0

    if prng is None:
        prng = Random()
        prng.seed(time())

    problem = inspyred.benchmarks.Rosenbrock(2)
    ea = inspyred.ec.ES(prng)
    ea.terminator = inspyred.ec.terminators.evaluation_termination
    final_pop = ea.evolve(generator=generator,
                          evaluator=evaluator,
                          pop_size=1,
                          bounder=bounder,
                          maximize=True,
                          max_evaluations=60)

    if display:
        best = max(final_pop)
        print('Best Solution: \n{0}'.format(str(best)))
    return ea

if __name__ == '__main__':
    # r = 1.08
    # n = 60
    # cur = 5
    # ser = []
    # ser_t = []
    # for i in range(n):
    #     ser.append(int(cur))
    #     ser_t.append(int(cur)/5*4)
    #     cur *= r


    # print(ser)
    # print(ser_t)
    # print("Total time needed: {} days".format(sum(ser_t)/60/24))
    main(display=True)
