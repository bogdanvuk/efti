import re
from multiprocessing import Pool, Process
import pexpect
import threading
import json

class EftiCmdBase:

    def __init__(self, fname):
        self.fname = fname
        self.res_raw = {}

    def __enter__(self):
        if self.fname:
            try:
                with open(self.fname) as data_file:
                    self.res_raw = json.load(data_file)
            except FileNotFoundError:
                self.res_raw = {}

            self.t = threading.Timer(10, self.dump_res)
            self.t.start()
            self.dump_res()

            return self

    def __exit__(self, type, value, traceback):
        if self.fname:
            self.dump_res()
            self.t.cancel()
            del self.t

    def dump_res(self):
        with open(self.fname, 'w') as outfile:
            json.dump(self.res_raw, outfile, indent = 4)

    def raw_cmd_handler_closure(self,name):
        def raw_cmd_handler(*args,**kwargs):
            cmd_name = name[len("cmd_"):]
            if cmd_name not in self.res_raw:
                self.res_raw[cmd_name] = []

            self.res_raw[cmd_name].append(kwargs)

        return raw_cmd_handler

    def __getattr__(self,name):
        if name.startswith("cmd_"):
            return self.raw_cmd_handler_closure(name)
    # Needed when passing objects of this class via multiprocessing module, which
    # tries to pickle them, and hence create an error because of overridden
    # __getattr__
    def __getstate__(self): return self.__dict__
    def __setstate__(self, d): self.__dict__.update(d)

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
    print('Running EFTI: {}', cmd_line)
    p = pexpect.spawnu(cmd_line, echo=True, timeout=3000)
    with cmd:
        try:
            while (1):
                p.expect('\n')
                #print(p.before)
                res_raw = cmd_decode(p.before + '\n')
                if res_raw:
                    if hasattr(cmd, 'cmd_' + res_raw['cmd']):
                        getattr(cmd, 'cmd_' + res_raw['cmd'])(*res_raw['args'], **res_raw['kwargs'])
        except pexpect.EOF:
            pass

    return cmd

def spawn_worker(kwargs):
    return spawn(**kwargs)

def spawn_group(cmd, path='./efti', params=[]):
    kwargs = [{'cmd': c, 'path':path, 'params':p} for c,p in zip(cmd, params)]
    with Pool(len(params)) as p:
        ret = p.map(spawn_worker, kwargs)

    return ret

def get_dsets_info(dsdir, regex):
    for fn in os.listdir(dsdir):
        fn = os.path.join(dsdir,fn)
        if os.path.isfile(fn):
            with open(fn) as f:
                res = re.findall(regex, f.read())
                if res:
                    yield os.path.splitext(os.path.basename(fn))[0], res[0]

def list_dsets_by_size(dsdir, constr):
    dsets = [d for d,i in get_dsets_info(dsdir, r"#define INST_CNT (\d*)")
             if constr(int(i))]

    return dsets

def efti_test(path, params, cmd_cls=EftiCmdBase, parallel=True, log=False):
    cmd = []
    fname = None
    for i in range(len(params)):
        if log:
            fname = "efti_test_log_w{}_{}.js".format(i, time.strftime("%Y%m%d_%H%M%S"))
        cmd.append(cmd_cls(fname))

    if parallel:
        cmd = spawn_group(cmd, path=path, params=params)
    else:
        for i in range(len(params)):
            cmd[i] = spawn(cmd[i], path=path, params=params[i])

    return cmd

def serial(cmd):
    pass
