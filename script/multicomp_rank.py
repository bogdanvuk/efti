from statsmodels.stats.multicomp import pairwise_tukeyhsd
import numpy as np

#algos = ['OC1-AP', 'OC1', 'CART-LC', 'OC1-SA', 'OC1-GA', 'OC1-ES', 'GALE', 'GaTree', 'HB-Lin']

def multicomp(data, algos):
    groups = []
    lindata = []
    for i,a in enumerate(algos):
        groups.extend([i]*len(data[a]))
        lindata.extend(data[a])

    groups = np.array(groups)
    lindata = np.array(lindata)
    res = pairwise_tukeyhsd(lindata, groups, 0.05)
    
    # print(res)
    return res

def multicomp_rank(data, desc=False):
    algos = list(sorted(data.keys()))
    res = multicomp(data, algos)

    avg = []
    for a in algos:
        avg.append(sum(data[a])/len(data[a]))
    
    num_classifiers = len(data)
    count = [0]*num_classifiers
    counter = 0
    cluster = [[0]*num_classifiers for n in range(num_classifiers)]
    
    for j in range(num_classifiers):
        for k in range(j+1, num_classifiers):
            if res.confint[counter, 0]*res.confint[counter, 1] <= 0:
                cluster[j][count[j]] = k;
                cluster[k][count[k]] = j;
                count[j] += 1;
                count[k] += 1;
                
            counter = counter + 1;

    ix = sorted(range(len(avg)), key=lambda k: avg[k], reverse=desc)
    b = sorted(avg, reverse=desc)
    already_ranked = [-1]*num_classifiers
    rank = [0]*num_classifiers
    rank_cur = 1
    counter = 0
    for j in range(num_classifiers):
        if not (ix[j] in already_ranked):
            rank[ix[j]] = rank_cur
            already_ranked[counter] = ix[j];
            counter += 1;
            for k in range(len(cluster[ix[j]])):
                if (cluster[ix[j]][k] != 0) and (not cluster[ix[j]][k] in already_ranked):
                    rank[cluster[ix[j]][k]] = rank_cur
                    already_ranked[counter] = cluster[ix[j]][k];
                    counter += 1;
            rank_cur += 1;

    rank = {name: rank[i] for i, name in enumerate(algos)}
       
    return rank
    
