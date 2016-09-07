
import sys

import os
import re
from efti_intf import efti_test, list_dsets_by_size
# from avg_fit_parallel import avg_fit

# def run_avg_fit(max_iter, sizes, dselect):

#     params = [ {
#         'max_iter': max_iter,
#         'ensemble_size': i,
#         'dataset_selection': dselect
#     } for i in sizes]

#     return avg_fit(path='../rel/efti', params=params, parallel=True, log=True)


if __name__ == "__main__":
    # all_ds = dsets_by_size('/data/projects/efti/src/datasets/', lambda x: True)
    max_iter = 50000
    # all_ds = dsets_by_size('/data/projects/efti/src/datasets/', lambda x: (x > 200) and (x <=500))
    # all_ds = list_dsets_by_size('/data/projects/efti/src/datasets/', lambda x: (x > 500))
    # all_ds.remove('wine')
    all_ds = ['adult','bank','bch','cvf','eb','eye','jvow','krkopt','letter','magic','mushroom','nurse','page','pen','shuttle','w21','wfr']
    # all_ds =["shuttle"]
    sizes = [1, 5, 9, 17]
    # sizes = [1, 5, 9]
    # sizes = [1]
    # sizes = [1, 4, 8, 16]
    params = [ {
        'max_iter': max_iter,
        'ensemble_size': i,
        'dataset_selection': ','.join(sorted(all_ds))
    } for i in sizes]

    efti_test(path='../rel/efti', params=params, log=True)
    # sizes = [1]
    # cmd = run_avg_fit(max_iter, sizes, ','.join(sorted(all_ds)))
    # res = {d:[0]*len(sizes) for d in all_ds}
    # for i,c in enumerate(cmd):
    #     for n,d in c.average().items():
    #         res[n][i] = d[1]

    # # print(res)
    # row_format ="{:>6}" + "{:>6.2f}" * (len(sizes))
    # for team, row in res.items():
    #     print(row_format.format(team, *row))
