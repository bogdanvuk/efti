import re
from multiprocessing import Pool, Process
import pexpect

def eval_arg_parse(*args, **kwargs):
    return args, kwargs

def cmd_decode(line):
    try:
        if line[0] == '$':
            result = re.search('\$(.*):', line)
            cmd_name = result.group(1)
            result = re.search(':(.*)\n', line)
            args, kwargs = eval('eval_arg_parse(' + result.group(1) + ')')
            return {'cmd': cmd_name, 'args': args, 'kwargs': kwargs}
        else:
            return None
    except Exception as e:
        print(str(e))
        print("Error decoding the command!")


def spawn(cmd, path='./efti', params={}):
    param_line = ['--{}={}'.format(k,v) for k,v in params.items()]
    cmd_line = path + ' ' + ' '.join(param_line)
    p = pexpect.spawnu(cmd_line, echo=True, timeout=300)
    
    try:
        while (1):
            p.expect('\n')
            #print(p.before)
            res = cmd_decode(p.before + '\n')
            if res:
                if hasattr(cmd, 'cmd_' + res['cmd']):
                    getattr(cmd, 'cmd_' + res['cmd'])(*res['args'], **res['kwargs'])
    except pexpect.EOF:
        pass

    return cmd

def spawn_worker(kwargs):
    return spawn(**kwargs)
    
def spawn_group(cmd, path='./efti', params=[]):
    kwargs = [{'cmd': c, 'path':path, 'params':p} for c,p in zip(cmd, params)]
    # jobs = []
    # for i in range(len(params)):
    #     p = Process(target=spawn, kwargs=kwargs[i])
    #     jobs.append(p)
    #     p.start()

    # for j in jobs:
    #     j.join()
        
    with Pool(len(params)) as p:
        ret = p.map(spawn_worker, kwargs)

    return ret

def serial(cmd):
    pass
