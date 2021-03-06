import re
from multiprocessing import Pool, Process
import pexpect
import threading
import json
import os
import time
import logging

class EftiCmdBase:

    def __init__(self, fname=None):
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
            # print("EXITING EftiCmdBase")
            self.dump_res()
            self.t.cancel()
            del self.t

    def dump_res(self):
        self.t.cancel()
        del self.t
        self.t = threading.Timer(10, self.dump_res)
        self.t.start()
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


def spawn(cmd, path='./efti', params=[], name='w0'):
    with cmd:
        for t in params:
            param_line = []
            for k,v in t.items():
                if v is not None:
                    if len(k) <= 2:
                        param_line.append('-{} {}'.format(k,v))
                    else:
                        param_line.append('--{}={}'.format(k,v))
                else:
                    if len(k) <= 2:
                        param_line.append('-{}'.format(k))
                    else:
                        param_line.append('--{}'.format(k))

            cmd_line = path + ' ' + ' '.join(param_line)
            logging.info('{}: Running EFTI: {}'.format(name, cmd_line))
            t = pexpect.spawnu(cmd_line, echo=True, timeout=None)
            try:
                while (1):
                    t.expect('\n')
                    logging.info('{}: {}'.format(name, t.before))
                    res_raw = cmd_decode(t.before + '\n')
                    if res_raw:
                        if hasattr(cmd, 'cmd_' + res_raw['cmd']):
                            getattr(cmd, 'cmd_' + res_raw['cmd'])(*res_raw['args'], **res_raw['kwargs'])
            except pexpect.EOF:
                pass

    return None

def spawn_worker(kwargs):
    return spawn(**kwargs)

def spawn_group(cmd, path='./efti', params=[]):
    kwargs = [{'cmd': c, 'path':path, 'params':t, 'name':'w{}'.format(i)} for i, (c,t) in enumerate(zip(cmd, params))]
    with Pool(len(params)) as t:
        ret = t.map(spawn_worker, kwargs)

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

def efti_test(path, tests=[], cmd_cls=EftiCmdBase, threads=0):
    cmd = []

    logging.basicConfig(format='%(asctime)s %(message)s', filename='efti_test.log', filemode='w', level = logging.DEBUG)
    logging.info('Starting EFTI test...')

    for t in tests:
        if 'log' in t:
            logfn = t['log']
        else:
            logfn = None

        cmd.append(cmd_cls(logfn))

    if threads == 0:
        for i,t in enumerate(tests):
            cmd[i] = spawn(cmd[i], path=path, params=t["conf"])
    else:
        logging.info('Using {} threads for {} tests'.format(threads, len(tests)))
        param_set_start = 0
        cmd_ret = []
        for param_set_start in range(0, len(tests), threads):
            logging.info("Starting tests {} to {}".format(param_set_start, param_set_start+threads-1))
            param_set = [t['conf'] for t in tests[param_set_start:param_set_start+threads]]
            cmd_set = cmd[param_set_start:param_set_start+threads]
            cmd_ret.extend(spawn_group(cmd_set, path=path, params=param_set))

        cmd = cmd_ret

    return cmd

def serial(cmd):
    pass
