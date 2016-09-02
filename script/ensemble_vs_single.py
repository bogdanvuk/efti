
import sys

import os
import re
from avg_fit_parallel import avg_fit

def run_avg_fit(max_iter, sizes):

    params = [ {
        'max_iter': max_iter,
        'ensemble_size': i,
        'dataset_selection': ','.join(sorted(dsets_by_size('/data/projects/efti/src/datasets/', lambda x: x > 20*i))
)
    } for i in sizes]

    return avg_fit(path='../rel/efti', params=params, parallel=True)

def dsets_by_size(dsdir, constr):
    dsets = []
    for fn in os.listdir(dsdir):
        fn = os.path.join(dsdir,fn)
        if os.path.isfile(fn):
            with open(fn) as f:
                res = re.findall(r"#define INST_CNT (\d*)", f.read())
                if res and constr(int(res[0])):
                    dsets.append(os.path.splitext(os.path.basename(fn))[0])

    return dsets


if __name__ == "__main__":
    all_ds = dsets_by_size('/data/projects/efti/src/datasets/', lambda x: True)

    max_iter = 500000
    sizes = [1, 5, 9, 17]
    # sizes = [1, 9]
    # sizes = [1, 4, 8, 16]

    # sizes = [1]
    cmd = run_avg_fit(max_iter, sizes)
    res = {d:[0]*len(sizes) for d in all_ds}
    for i,c in enumerate(cmd):
        for n,d in c.average().items():
            res[n][i] = d[1]

    # print(res)
    row_format ="{:>6}" + "{:>6.2f}" * (len(sizes))
    for team, row in res.items():
        print(row_format.format(team, *row))
