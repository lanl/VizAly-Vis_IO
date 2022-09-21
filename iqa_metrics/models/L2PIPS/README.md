# L<sup>2</sup>PIPS: Learn to Learn a Perceptual Image Patch Similarity Metric

[NTIRE 2021](https://data.vision.ee.ethz.ch/cvl/ntire21/) Perceptual Image Quality Assessment Challenge Submission

Our method ranked the third in the main score (SRCC + PLCC).

## Training 

```
python IQA_v9.py -opt settings/train/IQA_v9.json
```

* You should modify the "IQA_v9.json" based on your own environment. 

## Testing

```
python GetResults.py -opt settings/test/IQA.json
```

* Again, please modify the "IQA.json" document before running. Load the [trained model](https://drive.google.com/drive/folders/1ClTLIOJrrXX5i_h-EZU9LkXEh5TDAAuc?usp=sharing) to get the exact results in the [challenge report](https://arxiv.org/abs/2105.03072).

## Acknowledgements
This code is built on [DBCNN-PyTorch](https://github.com/zwx8981/DBCNN-PyTorch) and [CFSNet](https://github.com/qibao77/CFSNet). We thank the authors for sharing their codes.
