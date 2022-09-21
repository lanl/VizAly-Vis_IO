#!/usr/bin/env python
# coding: utf-8
import os
import torch
from skimage.io import imread
import cv2

img_orig_path = "./test_imgs/Reference"
img_test_path = "./test_imgs/Test"
img_diff_path = "./imgDiff"

def img_diff(orig, decomp, diff):
    # load images
    image1 = cv2.imread(orig) # reference image
    image2 = cv2.imread(decomp) # test image generated from decompressed dataset

    # compute difference
    difference = cv2.absdiff(image1, image2)

    # color the mask red
    Conv_hsv_Gray = cv2.cvtColor(difference, cv2.COLOR_BGR2GRAY)
    ret, mask = cv2.threshold(Conv_hsv_Gray, 0, 255, cv2.THRESH_BINARY_INV |cv2.THRESH_OTSU)
    difference[mask != 255] = [0, 255, 0]

    # add the red mask to the images to make the differences obvious
    image2[mask != 255] = [0, 255, 0]

    # store images
    cv2.imwrite(diff, image2)
    
def get_imgdiff(compressor, field_name):

    orig_name = os.path.join(img_orig_path, "NVB_C009_l10n512_S12345T692_z54_{}.png".format(field_name))
    
    img_list = [os.path.join(img_test_path, "{}-{}_config1.png".format(field_name, compressor)),
                os.path.join(img_test_path, "{}-{}_config2.png".format(field_name, compressor)),
                os.path.join(img_test_path, "{}-{}_config3.png".format(field_name, compressor))]
    
    c = 0
    for img_dist in img_list:
        c = c + 1
        diff_name = os.path.join(img_diff_path, "imgdiff_{}_{}_config-{}.png".format(compressor, field_name, c))
        img_diff(orig_name, img_dist, diff_name)

if __name__ == "__main__":
    for compressor in ["sz", "zfp", "mgard"]:
        for field_name in ["baryon_density", "dark_matter_density", "temperature", "velocity_x"]:
            get_imgdiff(compressor, field_name)


