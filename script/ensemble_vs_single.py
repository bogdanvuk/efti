
import sys

print(sys.executable)
import os
import re
from avg_fit_parallel import avg_fit

def run_avg_fit(max_iter, sizes, dsets):

    params = [ {'max_iter': max_iter, 'ensemble_size': i} for i in sizes]
    avg_fit(path='../rel/efti', params=params, parallel=False)

def dsets_by_size(dsdir, constr):
    dsets = []
    print(os.listdir(dsdir))
    for fn in os.listdir(dsdir):
        fn = os.path.join(dsdir,fn)
        if os.path.isfile(fn):
            print(fn)
            with open(fn) as f:
                res = re.findall(r"#define INST_CNT (\d*)", f.read())
                if res and constr(int(res[0])):
                    dsets.append(os.path.splitext(os.path.basename(fn))[0])

    return dsets


if __name__ == "__main__":
    ds = dsets_by_size('/data/projects/efti/src/datasets/', lambda x: x > 300)

    max_iter = 10000
    #sizes = [1, 16, 25, 32]
    # sizes = [1, 4, 8, 16]

    sizes = [9, 17]
    run_avg_fit(max_iter, sizes, ds)

