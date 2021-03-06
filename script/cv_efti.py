import sys
import time

from efti_intf import efti_test




files_all = ['adult', 'ausc', 'bank', 'bc', 'bch', 'bcw', 'ca', 'car', 'cmc', 'ctg', 'cvf', 'eb', 'eye', 'ger', 'gls', 'hep', 'hrtc', 'hrts', 'ion', 'irs', 'jvow', 'krkopt', 'letter', 'liv', 'lym', 'magic', 'msh', 'nurse', 'page', 'pen', 'pid', 'psd', 'sb', 'seg', 'shuttle', 'sick', 'son', 'spect', 'spf', 'thy', 'ttt', 'veh', 'vene', 'vote', 'vow', 'w21', 'w40', 'wfr', 'wilt', 'wine', 'zoo']

param_def = {
    'max_iter': 500000,
    'ensemble_size': 1,
    'dataset_selection': ','.join(sorted(files_all))
}

tests_all = [
    {
        'search_prob': 0.01,
        's_accel_stagn': 0.001,
        'return_prob': 5e-7,
        'weight_mut': 0.01,
        'topo_mut': 0.002
    },
    {
        'search_prob': 0.01,
        's_accel_stagn': 0.001,
        'return_prob': 0,
        'weight_mut': 0.01,
        'topo_mut': 0.002
    },
    {
        'search_prob': 0.05,
        's_accel_stagn': 0.001,
        'return_prob': 5e-7,
        'weight_mut': 0.01,
        'topo_mut': 0.002
    },
    {
        'search_prob': 0.001,
        's_accel_stagn': 0.001,
        'return_prob': 5e-7,
        'weight_mut': 0.01,
        'topo_mut': 0.002
    },
    {
        'search_prob': 0.01,
        's_accel_stagn': 0,
        'return_prob': 5e-7,
        'weight_mut': 0.01,
        'topo_mut': 0.002
    },
    {
        'search_prob': 0.01,
        's_accel_stagn': 0.001,
        'return_prob': 5e-7,
        'weight_mut': 0.1,
        'topo_mut': 0.002
    },
]

def run_tests(tests, threads=0):
    params = []
    for i,t in enumerate(tests):
        logfn = '{}_conf{}.js'.format(time.strftime("%Y%m%d_%H%M%S"), i)
        test_set = {'log': logfn, 'conf': []}

        p = param_def.copy()
        p.update({k:v for k,v in t.items()})
        test_set['conf'].append(p)

        params.append(test_set)

    efti_test(path='../rel/efti', threads=threads, tests=params)

if __name__ == "__main__":
   run_tests(tests_all, threads=3)
