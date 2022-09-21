import csv
import os.path
import random

import numpy as np
import scipy.io
import torch
import torchvision
from torch.utils.data import Dataset

# from .util import *
from data.util import default_loader, read_img, augment, get_image_paths


class PIPALFolder(Dataset):
    def __init__(self, root=None, index=None, transform=None, opt=None):

        if index is None:
            index = list(range(0, 200))
        if opt is not None:
            self.opt = opt
            root = opt['datasets']['pipal']
            patch_num = opt['patch_num']
        else:
            patch_num = 32

        refpath = os.path.join(root, 'Train_Ref')
        refname = self.getFileName(refpath, '.bmp')
        dispath = os.path.join(root, 'Train_Dis')
        txtpath = os.path.join(root, 'Train_Label')

        sample = []
        for i, item in enumerate(index):
            ref = refname[item]
            # print(ref, end=' ')
            txtname = ref.split('.')[0] + '.txt'
            fh = open(os.path.join(txtpath, txtname), 'r')
            for line in fh:
                line = line.split('\n')
                words = line[0].split(',')
                for aug in range(patch_num):
                    sample.append((
                        (os.path.join(dispath, words[0]), os.path.join(refpath, ref)),
                        np.array(words[1]).astype(np.float32) / 1000.0
                    ))
        # print('')

        self.samples = sorted(sample)
        self.transform = torchvision.transforms.Normalize(mean=[0.485, 0.456, 0.406],
                                                          std=[0.229, 0.224, 0.225])
        self.patch_size = opt['patch_size']
        # self.loader = default_loader

    def __getitem__(self, index):
        path, target = self.samples[index]
        '''img_dis = self.loader(path[0])
        img_ref = self.loader(path[1])'''
        img_dis = read_img(env=None, path=path[0])
        img_ref = read_img(env=None, path=path[1])

        '''if self.transform is not None:
            img_dis = self.transform(img_dis)
            img_ref = self.transform(img_ref)'''

        if self.patch_size < 288:
            H, W, _ = img_ref.shape
            crop_size = self.patch_size
            rnd_h = random.randint(0, max(0, (H - crop_size)))
            rnd_w = random.randint(0, max(0, (W - crop_size)))
            img_dis = img_dis[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]
            img_ref = img_ref[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]

        # augmentation - flip, rotate
        img_dis, img_ref = augment([img_dis, img_ref], self.opt['use_flip'], rot=False)

        if img_ref.shape[2] == 3:
            img_ref = img_ref[:, :, [2, 1, 0]]
            img_dis = img_dis[:, :, [2, 1, 0]]

        img_ref = torch.from_numpy(np.ascontiguousarray(np.transpose(img_ref, (2, 0, 1)))).float()
        img_dis = torch.from_numpy(np.ascontiguousarray(np.transpose(img_dis, (2, 0, 1)))).float()

        img_dis = self.transform(img_dis)
        img_ref = self.transform(img_ref)

        return {'Dis': img_dis, 'Ref': img_ref, 'Label': target}

    def __len__(self):
        length = len(self.samples)
        return length

    @staticmethod
    def getFileName(path, suffix):
        filename = []
        f_list = os.listdir(path)
        # print f_list
        for i in f_list:
            if os.path.splitext(i)[1] == suffix:
                filename.append(i)
        filename.sort()
        return filename


# TODO
class IQATestDataset(Dataset):

    def __init__(self, opt):
        super(IQATestDataset, self).__init__()
        self.opt = opt
        self.paths_Dis = None
        self.paths_Ref = None

        refpath = os.path.join(root, 'Train_Ref')
        refname = self.getFileName(refpath, '.bmp')
        dispath = os.path.join(root, 'Train_Dis')
        txtpath = os.path.join(root, 'Train_Label')

        sample = []
        for i, item in enumerate(index):
            ref = refname[item]
            # print(ref, end=' ')
            txtname = ref.split('.')[0] + '.txt'
            fh = open(os.path.join(txtpath, txtname), 'r')
            for line in fh:
                line = line.split('\n')
                words = line[0].split(',')
                sample.append((
                    (os.path.join(dispath, words[0]), os.path.join(refpath, ref)),
                    np.array(words[1]).astype(np.float32)
                ))
        # print('')

        self.samples = sample
        self.transform = torchvision.transforms.Normalize(mean=[0.485, 0.456, 0.406],
                                                          std=[0.229, 0.224, 0.225])

    def __getitem__(self, index):
        path, target = self.samples[index]

        img_dis = read_img(env=None, path=path[0])
        img_ref = read_img(env=None, path=path[1])

        '''H, W, _ = img_ref.shape
        crop_size = 224
        rnd_h = random.randint(0, max(0, (H - crop_size) // 2))
        rnd_w = random.randint(0, max(0, (W - crop_size) // 2))
        img_dis = img_dis[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]
        img_ref = img_ref[rnd_h:rnd_h + crop_size, rnd_w:rnd_w + crop_size, :]

        # augmentation - flip, rotate
        img_dis, img_ref = augment([img_dis, img_ref], self.opt['use_flip'], rot=False)'''

        if img_ref.shape[2] == 3:
            img_ref = img_ref[:, :, [2, 1, 0]]
            img_dis = img_dis[:, :, [2, 1, 0]]

        img_ref = torch.from_numpy(np.ascontiguousarray(np.transpose(img_ref, (2, 0, 1)))).float()
        img_dis = torch.from_numpy(np.ascontiguousarray(np.transpose(img_dis, (2, 0, 1)))).float()

        img_dis = self.transform(img_dis)
        img_ref = self.transform(img_ref)

        return {'Dis': img_dis, 'Ref': img_ref, 'Label': target, 'Dis_path': path[0]}

    def __len__(self):
        return len(self.samples)

    @staticmethod
    def getFileName(path, suffix):
        filename = []
        f_list = os.listdir(path)
        # print f_list
        for i in f_list:
            if os.path.splitext(i)[1] == suffix:
                filename.append(i)
        filename.sort()
        return filename

