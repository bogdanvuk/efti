#!/usr/bin/env python3
import json
import operator
def load_js_data(fname):
    with open(fname) as data_file:
        res = json.load(data_file)

    return res

def load_data(fn):
    res = load_js_data(fn)
    timings = {}
    for r in res['cv_pc_run']:
        ds = r['dataset']

        if ds not in timings:
            timings[ds] = 0

        timings[ds] += r['time']

    return timings

def balance(fn, threads):
    timings = load_data(fn)
    # print(';'.join(['{:.1f}'.format(timings[ds]) for ds in sorted(timings)]))
    # print(timings)
    total_time = sum(timings.values())
    avg_time = total_time / threads

    threads_t = [0]*threads
    threads_ds = [[] for t in range(threads)]

    while (timings):
        ds, t = max(timings.items(), key=operator.itemgetter(1))
        thread, _ = min(enumerate(threads_t), key=operator.itemgetter(1))
        threads_t[thread] += t
        threads_ds[thread].append(ds)
        del timings[ds]

    # thread_ds_start = [sorted(timings.keys())[0]]
    # thread_ds_id_start = [0]
    # thread_timings = []
    # cur_thread = 0
    # cur_time = 0

    # for i, ds in enumerate(sorted(timings)):
    #     t = timings[ds]
    #     if cur_time + t > avg_time:
    #         print("Cur_time: {}, t: {}, avg_time: {}".format(cur_time, t, avg_time))
    #         if (cur_thread < threads - 1) and (cur_time + t - avg_time < 0.5*t):
    #             cur_thread += 1
    #             thread_timings.append(cur_time)
    #             thread_ds_id_start.append(i)
    #             thread_ds_start.append(ds)
    #             cur_time = 0
    #     cur_time += t

    # thread_timings.append(cur_time)

    print("Avg time: {}".format(avg_time))
    print("Thread ds: {}".format(threads_ds))
    print("Thread timings: {}".format(threads_t))

if __name__ == "__main__":
    import sys

    balance(sys.argv[1], int(sys.argv[2]))
