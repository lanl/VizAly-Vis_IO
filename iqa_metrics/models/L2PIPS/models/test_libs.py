from models.modules.architecture import *
from models.modules.attns import MultiSpectralAttentionLayer, CALayer
from models.modules.common import *


class v5(torch.nn.Module):
    def __init__(self):
        nn.Module.__init__(self)

        res = resnet50_backbone(pretrained=True)

        self.head = nn.Sequential(
            res.conv1,
            res.bn1,
            res.relu,
            res.maxpool
        )
        # self.head = res.pre

        self.layer1 = res.layer1    # 128
        self.layer2 = res.layer2    # 256
        self.layer3 = res.layer3    # 512
        self.layer4 = res.layer4    # 512

        # self.avgpool = nn.AdaptiveAvgPool2d(1)
        # self.fc_res = res.fc


        nc = 256
        self.project1 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 56, 56), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 28, 28), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 14, 14), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )

        nc = 512
        self.project2 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 28, 28), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 14, 14), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )

        nc = 1024
        self.project3 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 14, 14), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )

        nc = 2048
        self.project4 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )

        num = (256 + 512 + 1024 + 2048) * 2 # + (128 + 256 + 512 + 512)
        self.big_fc = nn.Sequential(
            default_Linear(num, num),
            default_Linear(num, 1)
        )

    def forward(self, x_dis, x_ref):

        x_dis = self.head(x_dis)
        x_ref = self.head(x_ref)

        x_dis = self.layer1(x_dis)
        x_ref = self.layer1(x_ref)
        cat1, c1 = self._getsim(x_dis, x_ref)
        cat1 = self.project1(cat1).squeeze()

        x_dis = self.layer2(x_dis)
        x_ref = self.layer2(x_ref)
        cat2, c2 = self._getsim(x_dis, x_ref)
        cat2 = self.project2(cat2).squeeze()

        x_dis = self.layer3(x_dis)
        x_ref = self.layer3(x_ref)
        cat3, c3 = self._getsim(x_dis, x_ref)
        cat3 = self.project3(cat3).squeeze()

        x_dis = self.layer4(x_dis)
        x_ref = self.layer4(x_ref)
        cat4, c4 = self._getsim(x_dis, x_ref)
        cat4 = self.project4(cat4).squeeze()

        # out = self.final_fc(torch.cat([c1,c2,c3,cat1,cat2,cat3], dim=1))
        out = self.big_fc(torch.cat([c1,c2,c3,c4,cat1,cat2,cat3,cat4], dim=1))

        return out

    def _getsim(self, x_dis, x_ref):
        catandrearranged = self._rearrange(x_dis, x_ref)
        x_dis = x_dis.view(x_dis.size(0), x_dis.size(1), -1)
        x_ref = x_ref.view(x_ref.size(0), x_ref.size(1), -1)
        cosineDist = torch.cosine_similarity(x_dis, x_ref, dim=2)

        return catandrearranged, cosineDist

    @staticmethod
    def _rearrange(x_dis, x_ref):

        def _shuffle(a, n):
            result=[]
            for i in range(len(a)):
                if i == n:
                    break
                result.append(a[i])
                result.append(a[i+n])
            return result

        C = x_dis.size(1)
        out = torch.cat([x_dis, x_ref], dim=1)
        idx = _shuffle(list(range(C*2)), C)
        out = out[:, idx, :, :]

        return out


class v9(torch.nn.Module):
    def __init__(self):
        nn.Module.__init__(self)

        res = resnet50_backbone(pretrained=True)

        self.head1 = res.conv1
        self.head2 = nn.Sequential(
            res.bn1,
            res.relu,
            res.maxpool
        )
        # self.head = res.pre

        self.layer1 = res.layer1    # 128
        self.layer2 = res.layer2    # 256
        self.layer3 = res.layer3    # 512
        self.layer4 = res.layer4    # 512

        # self.avgpool = nn.AdaptiveAvgPool2d(1)
        # self.fc_res = res.fc

        nc = 64
        self.project0 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 112, 112), nn.PReLU(nc),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.PReLU(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 112, 112), nn.PReLU(nc),

            nn.Conv2d(nc, nc, 112, groups=nc, bias=False)
        )

        nc = 256
        self.project1 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 56, 56), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 28, 28), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 14, 14), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )
        # self.weight1 = nn.Sequential(
        #     nn.Conv2d(512, nc * 2, 3, 1, 1),
        #     nn.AdaptiveAvgPool2d(1)
        # )
        nc = 512
        self.project2 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 28, 28), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 14, 14), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )
        # self.weight2 = nn.Sequential(
        #     nn.Conv2d(512, nc * 2, 3, 1, 1),
        #     nn.AdaptiveAvgPool2d(1)
        # )
        nc = 1024
        self.project3 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 14, 14), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 2, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )
        # self.weight3 = nn.Sequential(
        #     nn.Conv2d(512, nc * 2, 3, 1, 1),
        #     nn.AdaptiveAvgPool2d(1)
        # )
        nc = 2048
        self.project4 = nn.Sequential(
            nn.Conv2d(nc * 2, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            nn.Conv2d(nc, nc, 3, 1, 1, groups=nc, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc), nn.ReLU(True),
            nn.Conv2d(nc, nc, 3, 1, 1, bias=False), nn.BatchNorm2d(nc),
            MultiSpectralAttentionLayer(nc, 7, 7), nn.ReLU(True),

            nn.Conv2d(nc, nc, 7, groups=nc, bias=False)
        )
        # self.weight4 = nn.Sequential(
        #     nn.Conv2d(512, nc * 2, 3, 1, 1),
        #     nn.AdaptiveAvgPool2d(1)
        # )

        num = (64 + 256 + 512 + 1024 + 2048) * 2 # + (128 + 256 + 512 + 512)
        self.big_fc = nn.Sequential(
            default_Linear(num, num),
            default_Linear(num, 1)
        )

    def forward(self, x_dis, x_ref):
        C = x_dis.size()[0]

        x_dis = self.head1(x_dis)
        x_ref = self.head1(x_ref)
        cat0, c0 = self._getsim(x_dis, x_ref)
        cat0 = self.project0(cat0).squeeze()
        if C == 1:
            cat0 = cat0.unsqueeze(dim=0)

        x_dis = self.head2(x_dis)
        x_ref = self.head2(x_ref)

        x_dis = self.layer1(x_dis)
        x_ref = self.layer1(x_ref)
        cat1, c1 = self._getsim(x_dis, x_ref)
        cat1 = self.project1(cat1).squeeze()
        if C == 1:
            cat1 = cat1.unsqueeze(dim=0)

        x_dis = self.layer2(x_dis)
        x_ref = self.layer2(x_ref)
        cat2, c2 = self._getsim(x_dis, x_ref)
        cat2 = self.project2(cat2).squeeze()
        if C == 1:
            cat2 = cat2.unsqueeze(dim=0)

        x_dis = self.layer3(x_dis)
        x_ref = self.layer3(x_ref)
        cat3, c3 = self._getsim(x_dis, x_ref)
        cat3 = self.project3(cat3).squeeze()
        if C == 1:
            cat3 = cat3.unsqueeze(dim=0)

        x_dis = self.layer4(x_dis)
        x_ref = self.layer4(x_ref)
        cat4, c4 = self._getsim(x_dis, x_ref)
        cat4 = self.project4(cat4).squeeze()
        if C == 1:
            cat4 = cat4.unsqueeze(dim=0)

        out = self.big_fc(torch.cat([c0,c1,c2,c3,c4,cat0,cat1,cat2,cat3,cat4], dim=1))

        return out

    def _getsim(self, x_dis, x_ref):
        catandrearranged = self._rearrange(x_dis, x_ref)
        x_dis = x_dis.view(x_dis.size(0), x_dis.size(1), -1)
        x_ref = x_ref.view(x_ref.size(0), x_ref.size(1), -1)
        cosineDist = torch.cosine_similarity(x_dis, x_ref, dim=2)

        return catandrearranged, cosineDist

    @staticmethod
    def _rearrange(x_dis, x_ref):

        def _shuffle(a, n):
            result=[]
            for i in range(len(a)):
                if i == n:
                    break
                result.append(a[i])
                result.append(a[i+n])
            return result

        C = x_dis.size(1)
        out = torch.cat([x_dis, x_ref], dim=1)
        idx = _shuffle(list(range(C*2)), C)
        out = out[:, idx, :, :]

        return out

