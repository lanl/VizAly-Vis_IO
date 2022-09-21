import argparse
import sys
import random

import torch
import torchvision
from torchvision import transforms
from PIL import Image
import os

import glob
import scipy.io as sio
# import pandas as pd
from tqdm import tqdm

sys.path.append('models/L2PIPS')

from data.util import is_image_file, read_img, augment, augment4rep
from models.test_libs import *
from settings import options as option
from utils import util
from utils.logger import PrintLogger
import numpy as np

img_orig_path = "./test_imgs/Reference"
img_test_path = "./test_imgs/Test"

def getDisRef(root):
    def _getFileName(path):
        filename = []
        for field_name in ["baryon_density", "dark_matter_density", "temperature", "velocity_x"]:
            for comp_name in ["sz", "zfp", "mgard"]:
                filename.extend(glob.glob("{}/{}-{}_config1.png".format(path, field_name, comp_name)))
                filename.extend(glob.glob("{}/{}-{}_config2.png".format(path, field_name, comp_name)))
                filename.extend(glob.glob("{}/{}-{}_config3.png".format(path, field_name, comp_name)))
        return filename

    pairs = []
    filenames = _getFileName(img_test_path)
    # print(filenames)
    for filename in filenames:
        refname = "NVB_C009_l10n512_S12345T692_z54_" + filename.split("/")[3].split('-')[0] + '.png'
        pairs.append(
            (os.path.join(filename),
             os.path.join(img_orig_path, refname))
        )

    return pairs

# pairs = getDisRef("./")
# print(pairs)

transform = torchvision.transforms.Normalize(mean=[0.485, 0.456, 0.406],
                                             std=[0.229, 0.224, 0.225])

parser = argparse.ArgumentParser()
# parser.add_argument('-opt', type=str, default='settings/test/IQA.json', help='Path to option JSON file.')
parser.opt = "models/L2PIPS/settings/test/IQA.json"
# opt = option.parse(parser.parse_args().opt)  # load settings and initialize settings
opt = option.parse(parser.opt)

# util.mkdir_and_rename(opt['path']['results_root'])
# util.mkdirs((path for key, path in opt['path'].items() if not key == 'saved_model'))
# opt = option.dict_to_nonedict(opt)

# # print to file and std_out simultaneously
# sys.stdout = PrintLogger(opt['path']['log'])
# # print('\n**********' + util.get_timestamp() + '**********')

device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
if opt['model_name'] == 'v9':
    model = torch.nn.DataParallel(v9()).to(device)
else:
    raise NotImplementedError(opt['model_name'])
model.load_state_dict(torch.load(opt['path']['saved_model'], map_location=torch.device('cpu')))
model.eval()
# print(model)

compute_pipal = True
if compute_pipal:

    val_root = opt['datasets']['pipal']
    pairs = getDisRef(val_root)

    # patch_num = opt['test_times']
    crop_size = opt['patch_size']
    reproducibility = True

    if reproducibility:    # for reproducibility, not random crop
        H = list(range(0, 64, 8))
        W = list(range(0, 64, 8))
        flips = [True, False]
        rots = [True, False]
        patch_num = 8 * 8 * 2 * 2
    else:
        raise NotImplementedError("Not reproducible!")

    f= open("output_L2PIPS.csv","w")
    f.write("dis_name"+','+"ref_name"+','+"score"+'\n')
    # scores = torch.zeros(len(pairs), 1)
    for idx, pfile in enumerate(pairs):
        dis_name = pfile[0]
        ref_name = pfile[1]

        dis = read_img(env=None, path=dis_name)
        ref = read_img(env=None, path=ref_name)
        score_ = []

        dis_tensor = torch.zeros([patch_num, 3, crop_size, crop_size], device=device)
        ref_tensor = torch.zeros([patch_num, 3, crop_size, crop_size], device=device)

        # for j in range(patch_num):
        j = 0
        for rot in rots:
            for flip in flips:
                for rnd_h in H:
                    for rnd_w in W:
                        I_dis = dis
                        I_ref = ref

                        # if crop_size < 288:
                            # H, W, _ = I_ref.shape
                            # rnd_h = random.randint(0, max(0, H - crop_size))
                            # rnd_w = random.randint(0, max(0, W - crop_size))
                            # I_dis = I_dis[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]
                            # I_ref = I_ref[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]

                        I_dis = I_dis[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]
                        I_ref = I_ref[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]

                        I_dis, I_ref = augment4rep([I_dis, I_ref], flip, rot)

                        if I_ref.shape[2] == 3:
                            I_ref = I_ref[:, :, [2, 1, 0]]
                            I_dis = I_dis[:, :, [2, 1, 0]]

                        I_ref = torch.from_numpy(np.ascontiguousarray(np.transpose(I_ref, (2, 0, 1)))).float()
                        I_dis = torch.from_numpy(np.ascontiguousarray(np.transpose(I_dis, (2, 0, 1)))).float()

                        I_dis = transform(I_dis)
                        I_ref = transform(I_ref)

                        I_dis = torch.unsqueeze(I_dis, dim=0)
                        I_ref = torch.unsqueeze(I_ref, dim=0)

                        I_dis = I_dis.to(device)
                        I_ref = I_ref.to(device)
                        # score_ = []

                        dis_tensor[j] = I_dis
                        ref_tensor[j] = I_ref
                        j = j + 1

        with torch.no_grad():
            score_tmp = model(dis_tensor, ref_tensor)
            score_tmp = torch.median(score_tmp.detach()).cpu().item()
            # score_.append(score_tmp)

        # score = round(score_tmp * 1000 * 10000) / 10000.0 # np.mean(score_)
        score = score_tmp # * 1000 # np.mean(score_)
        # std = std.cpu().item()
        # print(dis_name.split('/')[-1], end=',')
        # print(score)
        # scores[idx, 0] = score
        #print(dis_name+','+ref_name+','+str(score)+'\n')
        f.write(dis_name.split("/")[-1]+','+ref_name.split("/")[-1]+','+str(score)+'\n')

    f.close() 

