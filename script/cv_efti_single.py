import sys
import time

from efti_intf import efti_test

files_all = ['adult', 'ausc', 'bank', 'bc', 'bch', 'bcw', 'ca', 'car', 'cmc', 'ctg', 'cvf', 'eb', 'eye', 'ger', 'gls', 'hep', 'hrtc', 'hrts', 'ion', 'irs', 'jvow', 'krkopt', 'letter', 'liv', 'lym', 'magic', 'mushroom', 'nurse', 'page', 'pen', 'pid', 'psd', 'sb', 'seg', 'shuttle', 'sick', 'son', 'spect', 'spf', 'thy', 'ttt', 'veh', 'vene', 'vote', 'vow', 'w21', 'w40', 'wfr', 'wilt', 'wine', 'zoo']

param_def = {
    'max_iter': 500000,
    'ensemble_size': 1,
    'oversize_w': 0.02,
    # 'dataset_selection': ','.join(sorted(files_all)),
    'search_prob': 0.01,
    's_accel_stagn': 0.001,
    'return_prob': 5e-7,
    'weight_mut': 0.01,
    'topo_mut': 0.002
}
test_partition = [['shuttle'], ['eb'], ['bank'], ['letter', 'cvf', 'seg', 'thy', 'veh', 'psd', 'son', 'pid', 'lym', 'zoo', 'irs'], ['krkopt', 'w40', 'ctg', 'spf', 'wilt', 'sb', 'ion', 'ca', 'vote', 'hrts', 'vene'], ['adult', 'magic', 'nurse', 'wfr', 'w21', 'page', 'vow', 'cmc', 'ger', 'ttt', 'gls', 'bcw', 'liv'], ['pen', 'bch', 'jvow', 'eye', 'mushroom', 'wine', 'sick', 'car', 'hrtc', 'ausc', 'bc', 'spect', 'hep']]

tests_all = [{'dataset_selection': ','.join(ds)} for ds in test_partition]
# tests_all = []
# test_chunks = 7
# cv_runs = 5
# chunk_size = len(files_all)//test_chunks
# for i in range(test_chunks-1):
#     tests_all.append({'dataset_selection': ','.join(files_all[i*chunk_size:(i+1)*chunk_size])})

# tests_all.append({'dataset_selection': ','.join(files_all[(test_chunks-1)*chunk_size:])})

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
   run_tests(tests_all, threads=7)
