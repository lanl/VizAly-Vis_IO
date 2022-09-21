import argparse
import sys
import time

import torch.nn.functional as F
from scipy import stats
from torch import nn
from torch.optim import lr_scheduler

from data import *
from models.modules.architecture import resnet50_backbone, resnet34_backbone
from models.modules.attns import MultiSpectralAttentionLayer
from models.modules.common import default_Linear
from settings import options as option
from utils import util
from utils.logger import PrintLogger


def load_network(load_path, network, strict=True):
    if isinstance(network, nn.DataParallel):
        network = network.module
    network.load_state_dict(torch.load(load_path), strict=strict)


def freeze_params(m):
    for param in m.parameters():
        param.requires_grad = False


class DebugIQA(torch.nn.Module):
    def __init__(self, options):
        nn.Module.__init__(self)

        res = resnet50_backbone(pretrained=True)

        self.head1 = res.conv1
        self.head2 = nn.Sequential(
            res.bn1,
            res.relu,
            res.maxpool
        )

        self.layer1 = res.layer1
        self.layer2 = res.layer2
        self.layer3 = res.layer3
        self.layer4 = res.layer4

        if options['fc']:
            freeze_params(self.head1)
            freeze_params(self.head2)
            freeze_params(self.layer1)
            freeze_params(self.layer2)
            freeze_params(self.layer3)
            freeze_params(self.layer4)

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

        num = (64 + 256 + 512 + 1024 + 2048) * 2
        self.big_fc = nn.Sequential(
            default_Linear(num, num),
            default_Linear(num, 1)
        )

    def forward(self, x_dis, x_ref):
        x_dis = self.head1(x_dis)
        x_ref = self.head1(x_ref)
        cat0, c0 = self._getsim(x_dis, x_ref)
        cat0 = self.project0(cat0).squeeze()

        x_dis = self.head2(x_dis)
        x_ref = self.head2(x_ref)

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


class IQAManager(object):
    def __init__(self, options):
        print('Preparing the network.')
        self._options = options
        self._device = torch.device('cuda' if options['gpu_ids'] is not None else 'cpu')
        torch.backends.cudnn.benchmark = True if options['gpu_ids'] is not None else False
        self.schedulers = []
        # Network.
        self._net = torch.nn.DataParallel(DebugIQA(options)).to(self._device)
        if not self._options['fc']:
            self._net.load_state_dict(torch.load(self._options['path']['fc_root']))
            print('FC model loaded.')
        else:
            print(self._net)

        # # print(self._net)
        self.print_network(self._net)
        # Criterion.
        # self._criterion = torch.nn.MSELoss().to(self._device)
        if self._options['fc']:
            self._criterion = torch.nn.L1Loss().to(self._device)
        else:
            self._criterion = torch.nn.MSELoss().to(self._device)
        self._criterion_weight = self._options['loss_iqa_weight']

        # Solver.
        if self._options['fc']:
            self._solver = torch.optim.Adam(
                filter(lambda p: p.requires_grad, self._net.module.parameters()),
                lr=self._options['base_lr'],
                # momentum=0.9,
                weight_decay=self._options['weight_decay']
            )
        else:
            self._solver = torch.optim.Adam(
                self._net.module.parameters(),
                lr=self._options['base_lr'],
                weight_decay=self._options['weight_decay']
            )

        if self._options['lr_scheme'] == 'MultiStepLR':
            self.schedulers.append(
                lr_scheduler.MultiStepLR(self._solver, self._options['lr_steps'], self._options['lr_gamma']))
        else:
            raise NotImplementedError('MultiStepLR learning rate scheme is enough.')

        # Dataset
        self._options['phase'] = 'train'
        train_set = create_dataset(self._options)
        self._train_loader = create_dataloader(train_set, self._options)

        self._options['phase'] = 'test'
        test_set = create_dataset(self._options)
        self._test_loader = create_dataloader(test_set, self._options)

    def train(self):
        """Train the network."""
        print('------Training------')
        best_srcc = 0.0
        best_plcc = 0.0
        best_epoch = None
        print('Epoch\tTime\tLearning_rate\tLoss_IQA\t', end='')
        print('Train_SRCC\tTest_SRCC\tTest_PLCC')

        start_time = time.time()
        for t in range(self._options['epochs']):
            epoch_loss = []
            pscores = []
            tscores = []
            num_total = 0
            for XX in self._train_loader:
                X = XX['Dis'].clone().detach().cuda()  # torch.tensor(X.cuda())
                y_score = XX['Label'].clone().detach().cuda()
                X_ref = XX['Ref'].clone().detach().cuda()

                self._solver.zero_grad()
                score = self._net(X, X_ref)
                loss = self._criterion_weight * self._criterion(score, y_score.view(len(score), 1).float().detach())
                epoch_loss.append(loss.item())

                num_total += y_score.size(0)
                pscores = pscores + score.squeeze(dim=1).cpu().tolist()
                tscores = tscores + y_score.cpu().tolist()
                # Backward pass.
                loss.backward()
                self._solver.step()

            train_srcc, _ = stats.spearmanr(pscores, tscores)
            test_srcc, test_plcc = self._consistency()
            if test_srcc > best_srcc:
                best_srcc = test_srcc
                best_epoch = t + 1
                print('*', end='')
                if self._options['fc']:
                    modelpath = self._options['path']['fc_root']
                else:
                    modelpath = self._options['path']['db_root']
                torch.save(self._net.state_dict(), modelpath)
            if test_plcc > best_plcc:
                best_plcc = test_plcc

            time_elapsed = time.time() - start_time
            start_time = time.time()

            print('%d\t%ds\t%.1e\t\t%4.3f\t\t' %
                  (t + 1, time_elapsed, self.get_current_learning_rate(),
                   sum(epoch_loss) / len(epoch_loss)), end='')
            print('%4.4f\t\t%4.4f\t\t%4.4f' % (train_srcc, test_srcc, test_plcc))

            self.update_learning_rate()

        print('Best at epoch %d, test srcc: %f, test plcc: %f' % (best_epoch, best_srcc, best_plcc))
        return best_srcc, best_plcc

    def _consistency(self):
        self._net.eval()

        final_pscores = []
        # final_tscores = []
        for i in range(0, self._options['test_times']):
            # num_total = 0
            pscores = []
            tscores = []
            for XX in self._test_loader:
                # Data.
                X = XX['Dis'].clone().detach().cuda()  # torch.tensor(X.cuda())
                y_score = XX['Label'].clone().detach().cuda()
                X_ref = XX['Ref'].clone().detach().cuda()

                # Prediction.
                with torch.no_grad():
                    score = self._net(X, X_ref)
                pscores = pscores + score.squeeze(dim=1).cpu().tolist()  # score[0].cpu().tolist()
                tscores = tscores + y_score.cpu().tolist()
                # num_total += y_score.size(0)
            final_pscores.append(pscores)
            # final_tscores.append(tscores)
        pscores = np.mean(final_pscores, axis=0)
        pscores = pscores.squeeze().tolist()
        # tscores = np.mean(final_tscores, axis=0)
        # tscores = tscores.tolist()
        test_srcc, _ = stats.spearmanr(pscores, tscores)
        test_plcc, _ = stats.pearsonr(pscores, tscores)

        self._net.train()
        return abs(test_srcc), abs(test_plcc)

    def update_learning_rate(self):
        for scheduler in self.schedulers:
            scheduler.step()

    def get_current_learning_rate(self):
        return self._solver.param_groups[0]['lr']

    @staticmethod
    def print_network(network):
        if isinstance(network, nn.DataParallel):
            network = network.module
        s = str(network)
        n = sum(map(lambda x: x.numel(), network.parameters()))  # numle: get number of all elements
        print('Number of parameters in G: {:,d}'.format(n))


def main():
    """The main function."""
    parser = argparse.ArgumentParser()
    parser.add_argument('-opt', type=str, required=True, help='Path to option JSON file.')
    options = option.parse(parser.parse_args().opt)  # load settings and initialize settings

    util.mkdir_and_rename(options['path']['results_root'])  # rename old experiments if exists
    util.mkdirs((path for key, path in options['path'].items()
                 if not key == 'experiments_root' and
                 not key == 'saved_model' and
                 not key == 'fc_root' and
                 not key == 'db_root'))
    option.save(options)
    options = option.dict_to_nonedict(options)  # Convert to NoneDict, which return None for missing key.

    # Redirect all writes to the "txt" file
    sys.stdout = PrintLogger(options['path']['log'])

    # # logger = Logger(opt)

    if options['manual_seed'] is not None:
        random.seed(options['manual_seed'])

    index = None
    if options['dataset'] == 'pipal':
        index = list(range(0, 200))
    else:
        raise NotImplementedError

    lr_backup = options['base_lr']
    lr_steps_backup = options['lr_steps']
    epochs_backup = options['epochs']
    srcc_all = np.zeros((1, options['epoch']), dtype=np.float)
    plcc_all = np.zeros((1, options['epoch']), dtype=np.float)

    print('Total epochs:' + str(options['epoch']))
    for i in range(0, options['epoch']):
        # randomly split train-test set
        random.shuffle(index)
        train_index = index[0:round(0.8 * len(index))]
        test_index = index[round(0.8 * len(index)):len(index)]
        options['train_index'] = train_index
        options['test_index'] = test_index

        # train the fully connected layer only
        print('[No.%d/%d] Training the FC layer...' % (i, options['epoch']))
        options['fc'] = True
        options['base_lr'] = lr_backup
        options['lr_steps'] = lr_steps_backup
        options['epochs'] = epochs_backup
        manager = IQAManager(options)
        best_srcc1, best_plcc1 = manager.train()

        # fine-tune all model
        print('[No.%d/%d] Fine-tune all model...' % (i, options['epoch']))
        options['fc'] = False
        options['base_lr'] = options['base_lr_step2']
        options['lr_steps'] = options['lr_steps_step2']
        options['epochs'] = options['epochs_step2']
        manager = IQAManager(options)
        best_srcc2, best_plcc2 = manager.train()

        srcc_all[0][i] = np.max([best_srcc1, best_srcc2])
        plcc_all[0][i] = np.max([best_plcc1, best_plcc2])
        # srcc_all[0][i] = best_srcc1
        # plcc_all[0][i] = best_plcc1

    srcc_mean = np.mean(srcc_all)
    srcc_median = np.median(srcc_all)
    plcc_mean = np.mean(plcc_all)
    plcc_median = np.median(plcc_all)
    print(srcc_all)
    print('average srcc:%4.4f' % srcc_mean)
    print('median srcc:%4.4f' % srcc_median)
    print(plcc_all)
    print('average plcc:%4.4f' % plcc_mean)
    print('median plcc:%4.4f' % plcc_median)
    print('--------------Finish! [' + options['name'] + ']--------------')
    util.mkdir_and_rename(options['path']['results_root'], ('done_{:.4f}'.format(srcc_median)))


if __name__ == '__main__':
    main()
