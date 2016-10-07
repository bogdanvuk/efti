#!/usr/bin/env python3
import sys
import os
import time

from efti_intf import efti_test

files_all = ['adult', 'ausc', 'bank', 'bc', 'bch', 'bcw', 'ca', 'car', 'cmc', 'ctg', 'cvf', 'eb', 'eye', 'ger', 'gls', 'hep', 'hrtc', 'hrts', 'ion', 'irs', 'jvow', 'krkopt', 'letter', 'liv', 'lym', 'magic', 'msh', 'nurse', 'page', 'pen', 'pid', 'psd', 'sb', 'seg', 'shuttle', 'sick', 'son', 'spect', 'spf', 'thy', 'ttt', 'veh', 'vene', 'vote', 'vow', 'w21', 'w40', 'wfr', 'wilt', 'wine', 'zoo']

param_def = {
    'max_iter': 20000,
    'ensemble_size': 1,
    'oversize_w': 0.02,
    'search_prob': 0,
    's_accel_stagn': 0,
    'w_accel_stagn': 0.0,
    't_accel_stagn': 0.0,
    'weight_mut': 0.0,
    's_accel_stagn': 5e-4,
    'search_prob': 0.05,
    'search_function': 3,
    'return_prob': 0.001
}

#test_partition = [['shuttle'], ['eb'], ['bank'], ['letter', 'cvf', 'seg', 'thy', 'veh', 'psd', 'son', 'pid', 'lym', 'zoo', 'irs'], ['krkopt', 'w40', 'ctg', 'spf', 'wilt', 'sb', 'ion', 'ca', 'vote', 'hrts', 'vene'], ['adult', 'magic', 'nurse', 'wfr', 'w21', 'page', 'vow', 'cmc', 'ger', 'ttt', 'gls', 'bcw', 'liv'], ['pen', 'bch', 'jvow', 'eye', 'msh', 'wine', 'sick', 'car', 'hrtc', 'ausc', 'bc', 'spect', 'hep']]

test_partition = [['eb'], ['letter', 'cvf', 'seg', 'thy', 'veh', 'psd', 'son', 'pid', 'lym', 'zoo', 'irs'], ['krkopt', 'w40', 'ctg', 'spf', 'wilt', 'sb', 'ion', 'ca', 'vote', 'hrts', 'vene'], ['adult', 'magic', 'nurse', 'wfr', 'w21', 'page', 'vow', 'cmc', 'ger', 'ttt', 'gls', 'bcw', 'liv'], ['pen', 'bch', 'jvow', 'eye', 'msh', 'wine', 'sick', 'car', 'hrtc', 'ausc', 'bc', 'spect', 'hep']]

# tests_all = [{'dataset_selection': ','.join(ds)} for ds in test_partition]
# tests_all = [{'dataset_selection': ','.join(ds)} for ds in [files_all]]

tests_all = [
    {'s_accel_stagn': 1e-4},
    {'s_accel_stagn': 5e-3},
    {'return_prob': 0},
    {'search_prob': 0.1},
    {'search_prob': 0.2},
    {'search_prob': 0.01},
]

def run_tests(tests, threads=0, res_dir='.'):
    params = []
    for i,t in enumerate(tests):
        logfn = os.path.join(res_dir, '{}_conf{}.js'.format(time.strftime("%Y%m%d_%H%M%S"), i))
        test_set = {'log': logfn, 'conf': []}

        p = param_def.copy()
        p.update({k:v for k,v in t.items()})
        test_set['conf'].append(p)

        params.append(test_set)

    efti_test(path='../rel/efti', threads=threads, tests=params)

    return params

if __name__ == "__main__":
   run_tests(tests_all, threads=3)
