import torch.utils.data
import torchvision

from .IQAFolders import *


def create_dataloader(dataset, dataset_opt, drop_last=False):
    phase = dataset_opt['phase']
    mode = dataset_opt['mode']
    if mode.find('IQA') > -1:
        if phase == 'train':
            batch_size = dataset_opt['batch_size']
            shuffle = dataset_opt['use_shuffle']
            num_workers = dataset_opt['n_workers']
            drop_last = True
        else:
            batch_size = dataset_opt['batch_size']
            shuffle = True
            num_workers = dataset_opt['n_workers']
            drop_last = drop_last

        return torch.utils.data.DataLoader(
            dataset, batch_size=batch_size, shuffle=shuffle, num_workers=num_workers, pin_memory=True,
            drop_last=drop_last)

    else:
        raise NotImplementedError


def create_dataset(dataset_opt, index=None):
    phase = dataset_opt['phase']
    mode = dataset_opt['mode']

    if mode.find('IQA') > -1:
        patch_size = dataset_opt['patch_size']
        if phase == 'train':
            index = dataset_opt['train_index']
            # transform = True
            print('Train pics: ', end='')
        elif phase == 'test':
            index = dataset_opt['test_index']
            # transform = False
            print('Test pics: ', end='')
        else:
            raise NotImplementedError('Phase [{:s}] is not recognized.'.format(dataset_opt['phase']))

        if dataset_opt['dataset'] == 'pipal':
            dataset = PIPALFolder(
                root=dataset_opt['datasets']['pipal'], index=index,
                transform=None, opt=dataset_opt
            )
        else:
            raise NotImplementedError

    else:
        raise NotImplementedError('Mode [{:s}] is not recognized.'.format(mode))

    print('Dataset [{:s} - {:s} - {:s}] is created. \n Length: {:d}.'.format(dataset.__class__.__name__,
                                                                             dataset_opt['dataset'],
                                                                             dataset_opt['phase'],
                                                                             dataset.__len__()))

    return dataset
