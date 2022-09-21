#!/usr/bin/env python
# coding: utf-8
import os
import pandas as pd
import torch
import piq
from skimage.io import imread

img_orig_path = "./test_imgs/Reference"
img_test_path = "./test_imgs/Test"
img_IQA_path = "./iqa"

def mse(x: torch.Tensor, y: torch.Tensor):
    return torch.mean(( (x - y) ** 2), dim = [1, 2, 3])

#  calculate image quality assessment metrics for a test image
def img_assessment(x, y):
    """ 
    x: test image,
    y: reference image
    """
    # load images
    x = torch.tensor(imread(x)).permute(2, 0, 1)[None, ...] / 255.
    y = torch.tensor(imread(y)).permute(2, 0, 1)[None, ...] / 255.

    if torch.cuda.is_available():
        # Move to GPU to make computaions faster
        x = x.cuda()
        y = y.cuda()

    # image mse
    mse_index: torch.Tensor = mse(x, y)

    # To compute PSNR as a measure, use lower case function from the library.
    psnr_index = piq.psnr(x, y, data_range=1., reduction='none')
    # print(f"PSNR index: {psnr_index.item():0.4f}")

    # To compute SSIM index as a measure, use lower case function from the library:
    ssim_index = piq.ssim(x, y, data_range=1.)
    # In order to use SSIM as a loss function, use corresponding PyTorch module:
    # ssim_loss: torch.Tensor = piq.SSIMLoss(data_range=1.)(x, y)
    # print(f"SSIM index: {ssim_index.item():0.4f}, loss: {ssim_loss.item():0.4f}")

    # To compute MS-SSIM index as a measure, use lower case function from the library:
    ms_ssim_index: torch.Tensor = piq.multi_scale_ssim(x, y, data_range=1.)
    # # In order to use MS-SSIM as a loss function, use corresponding PyTorch module:
    # ms_ssim_loss = piq.MultiScaleSSIMLoss(data_range=1., reduction='none')(x, y)
    # print(f"MS-SSIM index: {ms_ssim_index.item():0.4f}, loss: {ms_ssim_loss.item():0.4f}")
    
    # To compute FSIM as a measure, use lower case function from the library
    fsim_index: torch.Tensor = piq.fsim(x, y, data_range=1., reduction='none')
    # # In order to use FSIM as a loss function, use corresponding PyTorch module
    # fsim_loss = piq.FSIMLoss(data_range=1., reduction='none')(x, y)
    # print(f"FSIM index: {fsim_index.item():0.4f}, loss: {fsim_loss.item():0.4f}")

    # To compute DSS as a measure, use lower case function from the library
    dss_index: torch.Tensor = piq.dss(x, y, data_range=1., reduction='none')
    # # In order to use DSS as a loss function, use corresponding PyTorch module
    # dss_loss = piq.DSSLoss(data_range=1., reduction='none')(x, y)
    # print(f"DSS index: {dss_index.item():0.4f}, loss: {dss_loss.item():0.4f}")

    # To compute SR-SIM score as a measure, use lower case function from the library:
    srsim_index: torch.Tensor = piq.srsim(x, y, data_range=1.)
    # # In order to use SR-SIM as a loss function, use corresponding PyTorch module:
    # srsim_loss: torch.Tensor = piq.SRSIMLoss(data_range=1.)(x, y)
    # print(f"SR-SIM index: {srsim_index.item():0.4f}, loss: {srsim_loss.item():0.4f}")

    # To compute HaarPSI as a measure, use lower case function from the library
    # This is port of MATLAB version from the authors of original paper.
    haarpsi_index: torch.Tensor = piq.haarpsi(x, y, data_range=1., reduction='none')
    # # In order to use HaarPSI as a loss function, use corresponding PyTorch module
    # haarpsi_loss: torch.Tensor = piq.HaarPSILoss(data_range=1., reduction='none')(x, y)
    # print(f"HaarPSI index: {haarpsi_index.item():0.4f}, loss: {haarpsi_loss.item():0.4f}")

    # To compute VSI score as a measure, use lower case function from the library:
    vsi_index: torch.Tensor = piq.vsi(x, y, data_range=1.)
    # # In order to use VSI as a loss function, use corresponding PyTorch module:
    # vsi_loss: torch.Tensor = piq.VSILoss(data_range=1.)(x, y)
    # print(f"VSI index: {vsi_index.item():0.4f}, loss: {vsi_loss.item():0.4f}")

    # To compute MDSI as a measure, use lower case function from the library
    mdsi_index: torch.Tensor = piq.mdsi(x, y, data_range=1., reduction='none')
    # # In order to use MDSI as a loss function, use corresponding PyTorch module
    # mdsi_loss: torch.Tensor = piq.MDSILoss(data_range=1., reduction='none')(x, y)
    # print(f"MDSI index: {mdsi_index.item():0.4f}, loss: {mdsi_loss.item():0.4f}")

    # To compute Multi-Scale GMSD as a measure, use lower case function from the library
    # It can be used both as a measure and as a loss function. In any case it should me minimized.
    # By default scale weights are initialized with values from the paper.
    # You can change them by passing a list of 4 variables to scale_weights argument during initialization
    # Note that input tensors should contain images with height and width equal 2 ** number_of_scales + 1 at least.
    ms_gmsd_index: torch.Tensor = piq.multi_scale_gmsd(
        x, y, data_range=1., chromatic=True, reduction='none')
    # # In order to use Multi-Scale GMSD as a loss function, use corresponding PyTorch module
    # ms_gmsd_loss: torch.Tensor = piq.MultiScaleGMSDLoss(
    #     chromatic=True, data_range=1., reduction='none')(x, y)
    # # print(f"MS-GMSDc index: {ms_gmsd_index.item():0.4f}, loss: {ms_gmsd_loss.item():0.4f}")

    # To compute LPIPS as a loss function, use corresponding PyTorch module
    lpips_loss: torch.Tensor = piq.LPIPS(reduction='none')(x, y)
    # print(f"LPIPS: {lpips_loss.item():0.4f}")

    # To compute DISTS as a loss function, use corresponding PyTorch module
    # By default input images are normalized with ImageNet statistics before forwarding through VGG16 model.
    # If there is no need to normalize the data, use mean=[0.0, 0.0, 0.0] and std=[1.0, 1.0, 1.0].
    dists_loss = piq.DISTS(reduction='none')(x, y)
    # print(f"DISTS: {dists_loss.item():0.4f}")

    # To compute PieAPP as a loss function, use corresponding PyTorch module:
    pieapp_loss: torch.Tensor = piq.PieAPP(reduction='none', stride=32)(x, y)
    # print(f"PieAPP loss: {pieapp_loss.item():0.4f}")

    return [mse_index.item(),   psnr_index.item(),    ssim_index.item(), ms_ssim_index.item(), fsim_index.item(),    dss_index.item(), 
            srsim_index.item(), haarpsi_index.item(), vsi_index.item(),  mdsi_index.item(),    ms_gmsd_index.item(), lpips_loss.item(), 
            dists_loss.item(), pieapp_loss.item()]

def get_img_metrics(compressor, field_name):
    """
    compressor: name of lossy compressor ("sz", "zfp", or "mgard")
    field_name: name of field of variables from Nyx applications used in this work ("baryon_density", "dark_matter_density", "temperature", "velocity_x")
    """
    
    img_orig = os.path.join(img_orig_path, "NVB_C009_l10n512_S12345T692_z54_{}.png".format(field_name))

    img_list = [os.path.join(img_test_path, "{}-{}_config1.png".format(field_name, compressor)),
                os.path.join(img_test_path, "{}-{}_config2.png".format(field_name, compressor)),
                os.path.join(img_test_path, "{}-{}_config3.png".format(field_name, compressor))]

    ImgName = []
    RelErr = []
    Mse_index = []
    Psnr_index = []
    Ssim_index = []
    Ms_ssim_index = []
    Fsim_index = []
    Dss_index = []
    Srsim_index = []
    Haarpsi_index = []
    Vsi_index = []
    Mdsi_index = []
    Ms_gmsd_index = []
    Lpips_loss = []
    Dists_loss = []
    Pieapp_loss = []
    
    c = 0
    for img_name in img_list:
        c = c + 1
        # shutil.copyfile(img_name, "vis_new/{}-{}_config{}.png".format(field_name, compressor, c))
        
        ImgName.append("{}-{}_config{}.png".format(field_name, compressor, c))
        RelErr.append("config{}".format(c))

        [mse_index,psnr_index,ssim_index,ms_ssim_index,fsim_index,dss_index,
         srsim_index,haarpsi_index,vsi_index,mdsi_index,ms_gmsd_index,lpips_loss,
         dists_loss,pieapp_loss] = img_assessment(img_name, img_orig)

        Mse_index.append(mse_index)
        Psnr_index.append(psnr_index)
        Ssim_index.append(ssim_index)
        Ms_ssim_index.append(ms_ssim_index)
        Fsim_index.append(fsim_index)
        Dss_index.append(dss_index)
        Srsim_index.append(srsim_index)
        Haarpsi_index.append(haarpsi_index)
        Vsi_index.append(vsi_index)
        Mdsi_index.append(mdsi_index)
        Ms_gmsd_index.append(ms_gmsd_index)
        Lpips_loss.append(lpips_loss)
        Dists_loss.append(dists_loss)
        Pieapp_loss.append(pieapp_loss)

    img_metrics_data = {
        "ImgName"      : ImgName,
        "RelErr"       : RelErr,
        "Mse"    : Mse_index,
        "PSNR"   : Psnr_index,
        "SSIM"   : Ssim_index,
        "MS-SSIM": Ms_ssim_index,
        "FSIM"   : Fsim_index,
        "DSS"    : Dss_index,
        "SRSIM"  : Srsim_index,
        "HaarPSI": Haarpsi_index,
        "VSI"    : Vsi_index,
        "MSDI"   : Mdsi_index,
        "MS-GMSD": Ms_gmsd_index,
        "LPIPS-VGG16"   : Lpips_loss,
        "DISTS"   : Dists_loss,
        "PieAPP"  : Pieapp_loss
        }
    
    img_metrics = pd.DataFrame(data = img_metrics_data)

    return img_metrics

if __name__ == "__main__":
    for comp_name in ["sz", "zfp", "mgard"]:
        for field_name in ["baryon_density", "dark_matter_density", "temperature", "velocity_x"]:
            img_metrics = get_img_metrics(comp_name, field_name)
            img_metrics.to_csv(os.path.join(img_IQA_path, "img_{}_{}.csv".format(comp_name, field_name)))

