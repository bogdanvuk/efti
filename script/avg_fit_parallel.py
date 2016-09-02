from efti_intf import spawn_group, spawn

def pp_avg(avg):
    avg_round = {}
    for d in sorted(avg):
        a = avg[d]
        avg_round[d] = ['{:.2f}'.format(val) for val in a]

    return avg_round

class EftiCmd:

    def __init__(self):
        self.res = {}
    
    def cmd_dataset(self, *args, **kwargs):
        name = kwargs['name']

        if name not in self.res:
            self.res[name] = []

    def cmd_cv_pc_run(self, *args, **kwargs):
        dataset = kwargs['dataset']
        res = {'fit': kwargs['fitness'], 'acc': kwargs['accuracy'], 'size': kwargs['nonleaves']}
        self.res[dataset].append(res)

        # print('Ensembles: {}, Avg: {}'.format(kwargs['ensemble_size'], pp_avg(self.average())))
        
    def average(self):
        avg = {}
        for n,d in self.res.items():
            fit = [r['fit'] for r in d]
            acc = [r['acc'] for r in d]
            size = [r['size'] for r in d]
            avg[n] = [sum(fit)/len(d), sum(acc)/len(d), sum(size)/len(d)]

        return avg

def avg_fit(path, params, parallel=True):
    cmd = []
    for i in range(len(params)):
        cmd.append(EftiCmd())
        #spawn(cmd[i], path=path, params=params[i])

    if parallel:
        cmd = spawn_group(cmd, path=path, params=params)
    else:
        for i in range(len(params)):
            cmd[i] = spawn(cmd[i], path=path, params=params[i])

    return cmd
    # print('*'*80)
    # print('*'*80)
    # for c,p in zip(cmd, params):
    #     print('Ensembles: {}, Avg: {}'.format(p['ensemble_size'], pp_avg(c.average())))
    # print('*'*80)

if __name__ == "__main__":

    max_iter = 10000
    #sizes = [1, 16, 25, 32]
    # sizes = [1, 4, 8, 16]

    sizes = [8, 16]
    params = [ {'max_iter': max_iter, 'ensemble_size': i} for i in sizes]
    avg_fit(path='../rel/efti', params=params, parallel=False)
