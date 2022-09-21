import math
import os
from datetime import datetime

import cv2
import matplotlib.pyplot as plt
import numpy as np
import torchvision.transforms as transforms
from skimage.metrics import structural_similarity
from torchvision.utils import make_grid


def imshow(tensor, title=None):
    image = tensor.cpu().clone()  # we clone the tensor to not do changes on it
    image = image.squeeze(0)  # remove the fake batch dimension
    unloader = transforms.ToPILImage()
    image = unloader(image)
    plt.imshow(image)
    if title is not None:
        plt.title(title)
    plt.pause(0.001)  # pause a bit so that plots are updated


#####################################################################
# file
######################################################################

def get_timestamp():
    return datetime.now().strftime('%y%m%d-%H%M%S')


def mkdir(path):
    if not os.path.exists(path):
        os.makedirs(path)


def mkdirs(paths):
    if isinstance(paths, str):
        mkdir(paths)
    else:
        for path in paths:
            mkdir(path)


def mkdir_and_rename(path, suffix=None):
    if suffix is not None:
        new_name = path + '_' + suffix
        os.rename(path, new_name)
    else:
        if os.path.exists(path):
            new_name = path + '_archived_' + get_timestamp()
            print('Path already exists. Rename it to [{:s}]'.format(new_name))
            os.rename(path, new_name)
        os.makedirs(path)


#####################################################################
# image
######################################################################
def tensor2img(tensor, out_type=np.uint8, min_max=(0, 1)):
    """
    Converts a torch Tensor into an image Numpy array
    Input: 4D(B,(3/1),H,W), 3D(C,H,W), or 2D(H,W), any range, RGB channel order
    Output: 3D(H,W,C) or 2D(H,W), [0,255], np.uint8 (default)
    """
    tensor = tensor.squeeze().float().cpu().clamp_(*min_max)  # clamp
    tensor = (tensor - min_max[0]) / (min_max[1] - min_max[0])  # to range [0,1]
    n_dim = tensor.dim()
    if n_dim == 4:
        n_img = len(tensor)
        img_np = make_grid(tensor, nrow=int(math.sqrt(n_img)), normalize=False).numpy()
        img_np = np.transpose(img_np[[2, 1, 0], :, :], (1, 2, 0))  # HWC, BGR
    elif n_dim == 3:
        img_np = tensor.numpy()
        img_np = np.transpose(img_np[[2, 1, 0], :, :], (1, 2, 0))  # HWC, BGR
    elif n_dim == 2:
        img_np = tensor.numpy()
    else:
        raise TypeError(
            'Only support 4D, 3D and 2D tensor. But received with dimension: {:d}'.format(n_dim))
    if out_type == np.uint8:
        img_np = (img_np * 255.0).round()
        # Important. Unlike matlab, numpy.unit8() WILL NOT round by default.
    return img_np.astype(out_type)


def save_img(img, img_path, mode='RGB'):
    cv2.imwrite(img_path, img, [int(cv2.IMWRITE_PNG_COMPRESSION), 0])


#####################################################################
# metric
######################################################################

def psnr(img1, img2):
    assert img1.dtype == img2.dtype == np.uint8, 'np.uint8 is supposed.'
    height1, width1 = img1.shape[:2]
    height2, width2 = img2.shape[:2]
    assert height1 == height2 and width1 == width2
    # if height1 > height2 and width1 > width2:
    #     shave_border = (height1 - height2) / 2
    #     img1 = img1[shave_border:height1-shave_border, shave_border:width1-shave_border]
    # elif height1 < height2 and width1 < width2:
    #     shave_border = (height2 - height1) / 2
    #     img2 = img2[shave_border:height2-shave_border, shave_border:width2-shave_border]
    img1 = img1.astype(np.float64)
    img2 = img2.astype(np.float64)
    mse = np.mean((img1 - img2) ** 2)
    if mse == 0:
        return float('inf')
    return 20 * math.log10(255.0 / math.sqrt(mse))


def rmse(img1, img2):
    assert img1.dtype == img2.dtype == np.uint8, 'np.uint8 is supposed.'
    img1 = img1.astype(np.float64)
    img2 = img2.astype(np.float64)
    mse = np.mean((img1 - img2) ** 2)
    if mse == 0:
        return float('inf')
    return math.sqrt(mse)


# def ssim(img1, img2, multichannel=False):
#     assert img1.dtype == img2.dtype == np.uint8, 'np.uint8 is supposed.'
#     return compare_ssim(img1, img2, multichannel=multichannel)


def ssim(img1, img2, multichannel=False):
    assert img1.dtype == img2.dtype == np.uint8, 'np.uint8 is supposed.'
    return structural_similarity(img1, img2, multichannel=multichannel)
