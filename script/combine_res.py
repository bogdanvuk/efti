#!/usr/bin/env python3
import json

def dump_js_data(fname, res):
    with open(fname, 'w') as outfile:
        json.dump(res, outfile, indent = 4)

def load_js_data(fname):
    with open(fname) as data_file:
        res = json.load(data_file)

    return res

def merge_res(res_in):
    res = {}
    for r in res_in:
        for k in r:
            if k not in res:
                res[k] = []

            res[k].extend(r[k])

    return res

def merge_files(fout, fins=[]):
    res_in = [load_js_data(f) for f in fins]
    res = merge_res(res_in)
    dump_js_data(fout,res)

if __name__ == '__main__':
    import sys
    merge_files(sys.argv[1], sys.argv[2:])
