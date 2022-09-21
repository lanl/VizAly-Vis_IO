from models.DeepQA.test_FR import IQANet, ToTensor, log_diff_fn
import os
import glob
import torch
import torchvision
import torch.nn as nn
import numpy as np
import matplotlib.image as img
from tqdm import tqdm

img_orig_path = "./test_imgs/Reference"
img_test_path = "./test_imgs/Test"

weight_file = "./models/DeepQA/FR_Sens_augment_best.pth"
result_score_txt = "./output_deepQA.csv"

def has_cuda():
    return torch.cuda.is_available()

if has_cuda():
    model = IQANet().cuda()
    model.load_state_dict(torch.load(weight_file))
    model.eval()
else:
    model = IQANet()
    model.load_state_dict(torch.load(weight_file, map_location=torch.device("cpu")))
    model.eval()

transforms = torchvision.transforms.Compose([ToTensor()])

filenames = []
for field_name in ["baryon_density", "dark_matter_density", "temperature", "velocity_x"]:
    for comp_name in ["sz", "zfp", "mgard"]:
        filenames.extend(glob.glob(os.path.join(img_test_path, "{}-{}_config1.png".format(field_name, comp_name))))
        filenames.extend(glob.glob(os.path.join(img_test_path, "{}-{}_config2.png".format(field_name, comp_name))))
        filenames.extend(glob.glob(os.path.join(img_test_path, "{}-{}_config3.png".format(field_name, comp_name))))

f = open(result_score_txt, "w")
f.write("test_name,ref_name,score\n")
for filename in tqdm(filenames): 
    d_img_name = os.path.join(filename)
    ext = os.path.splitext(d_img_name)[-1]
     
    if ext == ".png":
        r_img_name = "NVB_C009_l10n512_S12345T692_z54_" + filename.split("/")[3].split("-")[0] + ".png"
        r_img_name = os.path.join(img_orig_path, r_img_name)
        r_img = img.imread(r_img_name)

        if np.max(r_img) > 1:
            r_img = np.array(r_img).astype("float32") / 255
        # r_img: H x W x C(=RGB) -> H x W (Grayscale) -> 1 x H x W
        r_img = 0.2989 * r_img[:, :, 0] + 0.5870 * r_img[:, :, 1] + 0.1140 * r_img[:, :, 2]

        d_img = img.imread(d_img_name)
        if np.max(d_img) > 1:
            d_img = np.array(d_img).astype("float32") / 255
        # d_img: H x W x C(=RGB) -> H x W (Grayscale) -> 1 x H x W
        d_img = 0.2989 * d_img[:, :, 0] + 0.5870 * d_img[:, :, 1] + 0.1140 * d_img[:, :, 2]

        err = log_diff_fn(r_img, d_img)

        r_img = r_img[None, :, :]
        d_img = d_img[None, :, :]
        err = err[None, :, :]

        if has_cuda():
            # r_img = transforms(r_img)
            # r_img = torch.tensor(r_img.cuda()).unsqueeze(0)
            r_img = transforms(r_img).cuda()
            r_img = r_img.clone().detach().unsqueeze(0)

            # d_img = transforms(d_img)
            # d_img = torch.tensor(d_img.cuda()).unsqueeze(0)
            d_img = transforms(d_img).cuda()
            d_img = d_img.clone().detach().unsqueeze(0)

            # err = transforms(err)
            # err = torch.tensor(err.cuda()).unsqueeze(0)
            err = transforms(err).cuda()
            err = err.clone().detach().unsqueeze(0)
        else:
            # r_img = transforms(r_img)
            # r_img = torch.tensor(r_img.cuda()).unsqueeze(0)
            r_img = transforms(r_img)
            r_img = r_img.clone().unsqueeze(0)

            # d_img = transforms(d_img)
            # d_img = torch.tensor(d_img.cuda()).unsqueeze(0)
            d_img = transforms(d_img)
            d_img = d_img.clone().unsqueeze(0)

            # err = transforms(err)
            # err = torch.tensor(err.cuda()).unsqueeze(0)
            err = transforms(err)
            err = err.clone().unsqueeze(0) 

        # Quality prediction
        pred, sens, percept_err = model(r_img, d_img, err)

        line = "%s,%s,%f\n" % (d_img_name.split("/")[3], filename.split("/")[3].split("-")[0]+".png", float(pred.item()))
        f.write(line)

f.close()
