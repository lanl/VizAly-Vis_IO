import os

import torch
import torchvision
import torch.nn as nn

import numpy as np
# import matplotlib.image as img

from tqdm import tqdm

class IQANet(nn.Module):
    def __init__(self):
        super(IQANet, self).__init__()
        self.maxpool = nn.MaxPool2d(kernel_size=2, stride=2, ceil_mode=True)

        self.conv_featmap_ref = nn.Sequential(
            nn.Conv2d(in_channels=1, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
        )
        self.conv_featmap_dis = nn.Sequential(
            nn.Conv2d(in_channels=1, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
        )
        self.conv_featmap_err = nn.Sequential(
            nn.Conv2d(in_channels=1, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
            nn.Conv2d(in_channels=32, out_channels=32, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
        )

        self.conv_concat = nn.Sequential(
            nn.Conv2d(in_channels=96, out_channels=128, kernel_size=1, padding=0, bias=True),
            nn.ReLU(),
        )

        self.down_1 = nn.Sequential(
            nn.Conv2d(in_channels=128, out_channels=128, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
        )

        self.down_2 = nn.Sequential(
            nn.Conv2d(in_channels=128, out_channels=128, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
        )

        self.same = nn.Sequential(
            nn.Conv2d(in_channels=128, out_channels=128, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
        )

        self.up_2 = nn.Sequential(
            nn.ConvTranspose2d(in_channels=128 + 128, out_channels=128, kernel_size=3, stride=2, padding=1,
                               output_padding=1, dilation=1),
            nn.ReLU(),
        )

        self.up_1 = nn.Sequential(
            nn.ConvTranspose2d(in_channels=128 + 128, out_channels=128, kernel_size=3, stride=2, padding=1,
                               output_padding=1, dilation=1),
            nn.ReLU(),
        )

        self.sens = nn.Sequential(
            nn.Conv2d(in_channels=128 + 128, out_channels=1, kernel_size=3, padding=1, bias=True),
            nn.ReLU(),
        )

    def forward(self, r_img, d_img, err):
        # r_img: N_batch x N_channel x Height x Width
        featmap_ref = self.conv_featmap_ref(r_img)
        featmap_dis = self.conv_featmap_dis(d_img)
        featmap_err = self.conv_featmap_err(err)

        featmap_concat = torch.cat((featmap_ref, featmap_dis, featmap_err), dim=1)
        featmap = self.conv_concat(featmap_concat)

        featmap_1 = self.down_1(featmap)
        featmap_1_down = self.maxpool(featmap_1)

        featmap_2 = self.down_2(featmap_1_down)
        featmap_2_down = self.maxpool(featmap_2)

        featmap_3 = self.same(featmap_2_down)
        featmap_3 = torch.cat((featmap_3, featmap_2_down), dim=1)

        featmap_4 = self.up_2(featmap_3)
        featmap_4 = torch.cat((featmap_4, featmap_1_down), dim=1)

        featmap_5 = self.up_1(featmap_4)
        featmap_5 = torch.cat((featmap_5, featmap), dim=1)

        sens = self.sens(featmap_5)

        percept_err = torch.mul(sens, err)
        pred_score = torch.mean(percept_err, dim=(2, 3))

        # pred_score = self.fc(pred_score)

        return pred_score, sens, percept_err

class ToTensor(object):
    def __call__(self, sample):
        return torch.from_numpy(sample)

def log_diff_fn(in_a, in_b, eps=0.1):
    diff = 255 * (in_a - in_b)
    log_diff = np.float32(np.log(diff ** 2 + eps))
    log_max = np.float32(2 * np.log(255))
    log_min = np.float32(np.log(eps))

    log_diff_norm = (log_max - log_diff) / (log_max - log_min)

    return log_diff_norm

# dirname = './Test_Images'
# weight_file = 'FR_Sens_augment_best.pth'
# result_score_txt = 'output.txt'

# model = IQANet().cuda()
# model.load_state_dict(torch.load(weight_file))
# model.eval()

# transforms = torchvision.transforms.Compose([ToTensor()])

# filenames = os.listdir(dirname)
# filenames.sort()
# f = open(result_score_txt, 'w')
# for filename in tqdm(filenames):
#     d_img_name = os.path.join(dirname, filename)
#     ext = os.path.splitext(d_img_name)[-1]
#     if ext == '.bmp':
#         r_img_name = filename[:-10] + '.bmp'
#         r_img_name = os.path.join(dirname, 'Reference', r_img_name)
#         r_img = img.imread(r_img_name)
#         if np.max(r_img) > 1:
#             r_img = np.array(r_img).astype('float32') / 255
#         # r_img: H x W x C(=RGB) -> H x W (Grayscale) -> 1 x H x W
#         r_img = 0.2989 * r_img[:, :, 0] + 0.5870 * r_img[:, :, 1] + 0.1140 * r_img[:, :, 2]

#         d_img = img.imread(d_img_name)
#         if np.max(d_img) > 1:
#             d_img = np.array(d_img).astype('float32') / 255
#         # d_img: H x W x C(=RGB) -> H x W (Grayscale) -> 1 x H x W
#         d_img = 0.2989 * d_img[:, :, 0] + 0.5870 * d_img[:, :, 1] + 0.1140 * d_img[:, :, 2]

#         err = log_diff_fn(r_img, d_img)

#         r_img = r_img[None, :, :]
#         d_img = d_img[None, :, :]
#         err = err[None, :, :]

#         # r_img = transforms(r_img)
#         # r_img = torch.tensor(r_img.cuda()).unsqueeze(0)
#         r_img = transforms(r_img).cuda()
#         r_img = r_img.clone().detach().unsqueeze(0)

#         # d_img = transforms(d_img)
#         # d_img = torch.tensor(d_img.cuda()).unsqueeze(0)
#         d_img = transforms(d_img).cuda()
#         d_img = d_img.clone().detach().unsqueeze(0)

#         # err = transforms(err)
#         # err = torch.tensor(err.cuda()).unsqueeze(0)
#         err = transforms(err).cuda()
#         err = err.clone().detach().unsqueeze(0)

#         # Quality prediction
#         pred, sens, percept_err = model(r_img, d_img, err)

#         line = "%s,%f\n" % (filename, float(pred.item()))
#         f.write(line)
# f.close()

