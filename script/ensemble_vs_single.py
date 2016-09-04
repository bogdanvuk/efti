
import sys

import os
import re
from avg_fit_parallel import avg_fit

def run_avg_fit(max_iter, sizes, dselect):

    params = [ {
        'max_iter': max_iter,
        'ensemble_size': i,
        'dataset_selection': dselect
    } for i in sizes]

    return avg_fit(path='../rel/efti', params=params, parallel=True, log=True)

def get_dsets_info(dsdir, regex):
    for fn in os.listdir(dsdir):
        fn = os.path.join(dsdir,fn)
        if os.path.isfile(fn):
            with open(fn) as f:
                res = re.findall(regex, f.read())
                if res:
                    yield os.path.splitext(os.path.basename(fn))[0], res[0]

def dsets_by_size(dsdir, constr):
    dsets = [d for d,i in get_dsets_info(dsdir, r"#define INST_CNT (\d*)")
             if constr(int(i))]

    # for fn in os.listdir(dsdir):
    #     fn = os.path.join(dsdir,fn)
    #     if os.path.isfile(fn):
    #         with open(fn) as f:
    #             res = re.findall(r"#define INST_CNT (\d*)", f.read())
    #             if res and constr(int(res[0])):
    #                 dsets.append(os.path.splitext(os.path.basename(fn))[0])

    return dsets


if __name__ == "__main__":
    # all_ds = dsets_by_size('/data/projects/efti/src/datasets/', lambda x: True)
    max_iter = 100000
    all_ds = dsets_by_size('/data/projects/efti/src/datasets/', lambda x: (x > 200) and (x <=500))
    # all_ds = dsets_by_size('/data/projects/efti/src/datasets/', lambda x: (x > 500))
    # all_ds.remove('wine')
    # all_ds =["shuttle"]
    # sizes = [1, 5, 9, 17]
    sizes = [1, 5, 9]
    # sizes = [1]
    # sizes = [1, 4, 8, 16]

    # sizes = [1]
    cmd = run_avg_fit(max_iter, sizes, ','.join(sorted(all_ds)))
    res = {d:[0]*len(sizes) for d in all_ds}
    for i,c in enumerate(cmd):
        for n,d in c.average().items():
            res[n][i] = d[1]

    # print(res)
    row_format ="{:>6}" + "{:>6.2f}" * (len(sizes))
    for team, row in res.items():
        print(row_format.format(team, *row))
