import json
import os
from collections import OrderedDict
from datetime import datetime


def get_timestamp():
    return datetime.now().strftime('%y%m%d-%H%M%S')


def parse(opt_path):
    # load settings
    json_str = ''
    with open(opt_path, 'r') as f:
        for line in f:
            line = line.split('//')[0] + '\n'
            json_str += line
    opt = json.loads(json_str, object_pairs_hook=OrderedDict)
    opt['timestamp'] = get_timestamp()
    # scale = opt['scale']

    # export CUDA_VISIBLE_DEVICES
    gpu_list = ','.join(str(x) for x in opt['gpu_ids'])
    os.environ['CUDA_VISIBLE_DEVICES'] = gpu_list
    print('export CUDA_VISIBLE_DEVICES=' + gpu_list)

    if opt['mode'] is not None and opt['mode'].find('IQA') > -1:
        results_root = os.path.join(opt['path']['root'], 'results', opt['name'])
        for key, path in opt['path'].items():
            if path and key in opt['path']:
                opt['path'][key] = os.path.expanduser(path)
        opt['path']['results_root'] = results_root
        opt['path']['log'] = results_root
        if opt['mode'] != 'IQA_test':
            opt['path']['fc_model'] = os.path.join(results_root, 'fc_models')
            opt['path']['db_model'] = os.path.join(results_root, 'db_models')
            opt['path']['fc_root'] = os.path.join(opt['path']['fc_model'], 'net_params_best.pkl')
            opt['path']['db_root'] = os.path.join(opt['path']['db_model'], 'net_params_best.pkl')

        return opt

    else:
        raise NotImplementedError


def save(opt):
    dump_dir = opt['path']['experiments_root'] if opt['is_train'] else opt['path']['results_root']
    dump_path = os.path.join(dump_dir, 'options.json')
    with open(dump_path, 'w') as dump_file:
        json.dump(opt, dump_file, indent=2)


class NoneDict(dict):
    def __missing__(self, key):
        return None


# convert to NoneDict, which return None for missing key.
def dict_to_nonedict(opt):
    if isinstance(opt, dict):
        new_opt = dict()
        for key, sub_opt in opt.items():
            new_opt[key] = dict_to_nonedict(sub_opt)
        return NoneDict(**new_opt)
    elif isinstance(opt, list):
        return [dict_to_nonedict(sub_opt) for sub_opt in opt]
    else:
        return opt
