import pickle
from efti_intf import EftiCmdBase, spawn_group, spawn
import time

def pp_avg(avg):
    avg_round = {}
    for d in sorted(avg):
        a = avg[d]
        avg_round[d] = ['{:.2f}'.format(val) for val in a]

    return avg_round

class EftiCmd(EftiCmdBase):

    def __init__(self,fname):
        self.res = {}
        super().__init__(fname)

    def cmd_dataset(self, *args, **kwargs):
        EftiCmdBase.__getattr__(self,'cmd_dataset')(*args, **kwargs)
        name = kwargs['name']

        if name not in self.res:
            self.res[name] = []

    def cmd_cv_pc_run(self, *args, **kwargs):
        EftiCmdBase.__getattr__(self,'cmd_cv_pc_run')(*args, **kwargs)
        dataset = kwargs['dataset']
        res = {'fit': kwargs['fitness'], 'acc': kwargs['accuracy'], 'size': kwargs['nonleaves']}
        self.res[dataset].append(res)
        print(kwargs)
        # print('Ensembles: {}, Avg: {}'.format(kwargs['ensemble_size'], pp_avg(self.average())))

    def average(self):
        avg = {}
        for n,d in self.res.items():
            fit = [r['fit'] for r in d]
            acc = [r['acc'] for r in d]
            size = [r['size'] for r in d]
            avg[n] = [sum(fit)/len(d), sum(acc)/len(d), sum(size)/len(d)]

        return avg

if __name__ == "__main__":

    max_iter = 10000
    #sizes = [1, 16, 25, 32]
    # sizes = [1, 4, 8, 16]

    sizes = [8, 16]
    params = [ {'max_iter': max_iter, 'ensemble_size': i} for i in sizes]
    avg_fit(path='../rel/efti', params=params, parallel=False)
