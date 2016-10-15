#!/usr/bin/env python3
import glob
import os
import math
import json
import csv
import numpy as np
import scipy as sp
import scipy.stats
from efti_intf import get_dsets_info
from multicomp_rank import multicomp_rank

features = {
    'acc':{'js_feat': 'accuracy', 'desc': True},
    'size':{'js_feat': 'leaves', 'desc': False},
    'time':{'js_feat': 'time', 'desc': False},
    'fit':{'js_feat': None, 'desc': True}
}

def mean_confidence_interval(data, confidence=0.95):
    a = 1.0*np.array(data)
    n = len(a)
    m, se = np.mean(a), scipy.stats.sem(a)
    h = se * sp.stats.t._ppf((1+confidence)/2., n-1)
    return m, h

def load_js_data(fname):
    with open(fname) as data_file:
        res = json.load(data_file)

    return res

def calc_data_fitness(acc, categs, size, complexity_weight):
    oversize = (size - categs)/categs
    return acc * (1 - complexity_weight*oversize*oversize)

def anova(data, desc=False):
    ranks = {}
    # for ds in sorted(data.keys()):
    for ds in data:
        # dict_data = {i:v for i,v in enumerate(data[ds])}
        if len(data[ds]) >= 2:
            # print('*'*20 + ' {} '.format(ds) + '*'*20)
            ranks[ds] = multicomp_rank(data[ds], desc=desc)

    return ranks

def form_mean_table(data):
    table = {}
    for ds in data:
        table[ds] = {}
        for algo, algo_data in data[ds].items():
            table[ds][algo] = mean_confidence_interval(algo_data)

    return table

import re
def natural_key(string_):
    """See http://www.codinghorror.com/blog/archives/001018.html"""
    return [int(s) if s.isdigit() else s for s in re.split(r'(\d+)', string_)]

def split_table_horizontally(table, splits):
    if splits == 1:
        rows = table
    else:
        body = table[1:]
        height = math.ceil(len(body)/splits);
        rows = [table[0]*splits]
        for h in range(height):
            row = []
            for s in range(splits):
                ielem = s*height + h
                if ielem < len(body):
                    row.extend(body[ielem])

            rows.append(row)

    return rows

def prepare_csv_table(table, cvs, dstitle='Dataset', head_fmt=r"{}", data_fmt="{0:0.2f}", sort_by_desc=True):
    if sort_by_desc:
        cvs_sort = list(sorted(cvs, key=lambda c: natural_key(cvs[c]['desc'])))
    else:
        cvs_sort = cvs

    head_row = [dstitle] + [head_fmt.format(cvs[c]['desc']) for c in cvs_sort]
    csv_table = [head_row]

    for d,res in iter(sorted(table.items())):
        row = [d]

        for a in cvs_sort:
            if a in res:
                try:
                    # If the result is a tuple containing the variance info
                    row.append(data_fmt.format(res[a][0], res[a][1]))
                except (TypeError, IndexError) as e:
                    row.append(data_fmt.format(res[a]))

            else:
                row += ["-"]

        csv_table.append(row)

    return csv_table

def write_csv_table(fn, table, horizontal_splits=1):
    with open(fn, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile, delimiter=',',
                               quotechar='"', quoting=csv.QUOTE_MINIMAL)

        rows = split_table_horizontally(table, horizontal_splits)

        for r in rows:
            csvwriter.writerow(r)

def dump_table_csv(fn, table, cvs, horizontal_splits=1, variance=False, sort_by_desc=True,
                   dstitle='Dataset', head_fmt=r"{}", data_fmt="{0:0.2f}"):

    csv_table = prepare_csv_table(table=table, cvs=cvs, dstitle=dstitle,
                                  head_fmt=head_fmt, data_fmt=data_fmt,
                                  sort_by_desc=sort_by_desc)
    write_csv_table(fn, csv_table, horizontal_splits=horizontal_splits)

def derive_relative_table(table):
    table_rel = {}
    for ds in table:
        table_rel[ds] = {}
        val_max = max(table[ds].values())
        for algo, val in table[ds].items():
            table_rel[ds][algo] = val[0]/val_max

    return table_rel

def form_mean_rank(rank):
    mean_rank = {}
    for ds in rank:
        for algo, val in rank[ds].items():
            if algo not in mean_rank:
                mean_rank[algo] = []

            mean_rank[algo].append(val)

    for algo in mean_rank:
        mean_rank[algo] = sum(mean_rank[algo])/len(mean_rank[algo])

    return mean_rank

def dump_mean_sorted_ranks(fn, ranks, cvs):
    with open(fn, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile, delimiter=',',
                               quotechar='"', quoting=csv.QUOTE_MINIMAL)

        for f,r in ranks.items():
            cvs_sort = list(sorted(cvs, key=lambda c: r[c]))
            csvwriter.writerow(['Feature']+[cvs[c]['desc'] for c in cvs_sort])

            row = [f]

            for a in cvs_sort:
                if a in r:
                    val = r[a]

                    row += ["{0:0.2f}".format(val)]
                else:
                    row += ["-"]

            csvwriter.writerow(row)
            csvwriter.writerow([])


def rank(files, complexity_weight):
    data, cvs = load_data(files, complexity_weight)
    # for k,a in cvs.items():
    #     if a['desc'] == "nodt":
    #         break

    # for ds in data['time']:
    #     if k in data['time'][ds]:
    #         for i in range(len(data['time'][ds][k])):
    #             data['time'][ds][k][i] *= 20

    mean_ranks = {}
    for f in features:
        table = form_mean_table(data[f])
        dump_table_csv("mean_{}.csv".format(f), table, cvs)

        table_rel = derive_relative_table(table)
        dump_table_csv("mean_relative_{}.csv".format(f), table_rel, cvs)

        # if f == "fit":
        rank = anova(data[f], features[f]['desc'])
        dump_table_csv("rank_{}.csv".format(f), rank, cvs)

        mean_ranks[f] = form_mean_rank(rank)

    dump_mean_sorted_ranks("mean_ranks.csv", mean_ranks, cvs)

def load_data(files, complexity_weight):
    categs = {
        d:int(c) for d,c in get_dsets_info(
            os.path.expandvars('$EFTI/src/datasets'),
            r"#define CATEG_MAX (\d*)"
        )
    }

    data = {f:{} for f in features}
    cvs = {}
    for i,f in enumerate(files):
        cvs[i] = {'fn': f, 'desc': str(i)}
        res = load_js_data(f)

        if 'desc' in res:
            cvs[i]['desc'] = res['desc']

        for r in res['cv_pc_run']:
            ds = r['dataset']

            for feat_table_name, feat in features.items():
                if ds not in data[feat_table_name]:
                    data[feat_table_name][ds] = {}

                if feat['js_feat'] in r:
                    if i not in data[feat_table_name][ds]:
                        data[feat_table_name][ds][i] = []

                    data[feat_table_name][ds][i].append(r[feat['js_feat']])

            if ('accuracy' in r) and ('leaves' in r):
                fit = calc_data_fitness(r['accuracy'], categs[ds], r['leaves'],
                                         complexity_weight=complexity_weight)
                if i not in data['fit'][ds]:
                    data['fit'][ds][i] = []

                data['fit'][ds][i].append(fit)

    return data, cvs

if __name__ == "__main__":
    import sys
    if len(sys.argv) > 2:
        complexity_weight = float(sys.argv[2])
    else:
        complexity_weight = 0.2

    if 'EFTI' not in os.environ:
        os.environ['EFTI'] = os.path.abspath('../')

    files = list(glob.iglob(os.path.join(sys.argv[1], '**/*.js'), recursive=True))
    rank(files, complexity_weight)
