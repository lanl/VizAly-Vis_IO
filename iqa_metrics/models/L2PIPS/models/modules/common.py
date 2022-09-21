import math

import torch
import torch.nn as nn
import torch.nn.functional as F


class FReLU(nn.Module):
    r""" FReLU formulation. The funnel condition has a window size of kxk. (k=3 by default)
    """
    def __init__(self, in_channels):
        super().__init__()
        self.conv_frelu = nn.Conv2d(in_channels, in_channels, kernel_size=3, stride=1, padding=1, groups=in_channels)
        self.bn_frelu = nn.BatchNorm2d(in_channels)

    def forward(self, x):
        y = self.conv_frelu(x)
        y = self.bn_frelu(y)
        x = torch.max(x, y)
        return x


def weight_init(net, nl='relu'):
    for m in net.modules():
        if isinstance(m, nn.Conv2d):
            nn.init.kaiming_normal_(m.weight.data, nonlinearity=nl)
            if m.bias is not None:
                m.bias.data.zero_()
        elif isinstance(m, nn.Linear):
            nn.init.kaiming_normal_(m.weight.data, nonlinearity=nl)
            if m.bias is not None:
                m.bias.data.zero_()
        elif isinstance(m, nn.BatchNorm2d):
            m.weight.data.fill_(1)
            if m.bias is not None:
                m.bias.data.zero_()


####################
# Basic blocks
####################
'''
def default_conv(in_channels, out_channels, kernel_size, padding, bias=False, init_scale=0.1):
    basic_conv = nn.Conv2d(in_channels, out_channels, kernel_size, padding=padding, bias=bias)
    nn.init.kaiming_normal_(basic_conv.weight.data, a=0, mode='fan_in')
    basic_conv.weight.data *= init_scale
    if basic_conv.bias is not None:
        basic_conv.bias.data.zero_()
    return basic_conv
'''


def default_conv(in_channels, out_channels, kernel_size, stride=1, padding=None, bias=True, init_scale=0.2):
    if padding is None:
        padding = kernel_size // 2
    basic_conv = nn.Conv2d(
        in_channels=in_channels, out_channels=out_channels,
        kernel_size=kernel_size, stride=stride,
        padding=padding, bias=bias)
    nn.init.kaiming_normal_(basic_conv.weight.data, a=0, mode='fan_in')
    basic_conv.weight.data *= init_scale
    if basic_conv.bias is not None:
        basic_conv.bias.data.zero_()
    return basic_conv


def default_Linear(in_channels, out_channels, bias=True, scale=1):
    basic_Linear = nn.Linear(in_channels, out_channels, bias=bias)
    # nn.init.xavier_normal_(basic_Linear.weight.data)
    nn.init.kaiming_normal_(basic_Linear.weight.data, a=0, mode='fan_in')
    basic_Linear.weight.data *= scale
    if basic_Linear.bias is not None:
        basic_Linear.bias.data.zero_()
    return basic_Linear

