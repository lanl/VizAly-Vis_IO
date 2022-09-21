# VizAly-Vis_IO: Framework for Visualization and Analysis of Simulation Data

## Project Scope
VizAly is a general framework for **A**na**ly**sis and **Vi**suali**z**ation of simulation data. As supercomputing resources increase, cosmological scientists are able to run more detailed and larger simulations generating massive amounts of data. Analyzing these simulations with an available open-source toolkit is important for collaborative Department of Energy scientific discovery across labs, universities, and other partners. Developed software as a part of this collection include: comparing data with other existing simulations, verifying and validating results with observation databases, new halo finder algorithms, and using analytical tools to get insights into the physics of the cosmological universe. The goal of this software project is to provide a set of open-source libraries, tools, and packages for large-scale cosmology that allows scientists to visualize, analyze, and compare large-scale simulation and observational data sets. Developed software will provide a variety of methods for processing, visualization, and analysis of astronomical observation and cosmological simulation data. These tools are intended for deployment on multiple scientific computing platforms, including but not limited to personal computers, cloud computing, experimental sites (telescopes) and high-performance supercomputers.

The focus of this repo is the Visualization and IO for HACC.

<!-- AD/AE for submission to DRBSD22: Analyzing the Impact of Lossy Data Reduction on Volume Rendering of Cosmology Data. -->
## Installation

<!-- **Reproductioin initiative** -->

[![DOI](https://zenodo.org/badge/528606976.svg)](https://zenodo.org/badge/latestdoi/528606976) [![DOI](https://zenodo.org/badge/528893694.svg)](https://zenodo.org/badge/latestdoi/528893694)

http://ieee-dataport.org/documents/nyx-cosmological-simulation-data

### Requirements 

- Python 3.8
- Pytorch 1.12.1
- Scikit-image 0.19.3
- Opencv-python 4.6.0.66
- Opencv-contrib-python 4.6.0.66
- Piq 0.7.0
- Lmdb 1.3.0
- Imageio 2.9.0
- Pillow 7.1.2

Required modules can be installed via ` pip3 install -r requirements.txt `


### Foresight configurations files for compression experiments

In compression experiments, we specify the configurations, especially the absolute error bound, for each compressor and each field into a JSON file. Those JSON files are contained in **foresight_input**. For example, `nyx_img_compression_sz_abs_baryon_density.json` indicates the configuration JSON file for SZ to compress *Baryon_density*.

```bash
./CBench -i nyx_img_compression_sz_abs_baryon_density.json
```

### ParaView scripts for volume rendering

The scripts to generate volume rendering visualization using ParaView are listed under **visualization_scripts**.

```bash
PYTHON ... img_baryon_density_ReOr.py
```

### Image difference computation

The `getImgDiff.py` is used to calculate the absolute difference between testing images and reference images, which can be found in **iqa_metrics**. The testing images are contained in **Test** and reference images are contained in **Reference**. The imgDiff images will be output to **imgDiff**.

```bash
cd iqa_metrics
mkdir -p imgDiff
python getImgDiff.py
```

### Image quality assessment (IQA) metrics computation

The scripts used to compute the IQA metrics can be found in **iqa_metrics**. All IQA metrics can be computed using `getIQA.py`, except L2PIPS and DeepQA. Calculated IQA metrics values are organized in **iqa**.

```bash
# for all the metrics beside L2PIPS and DeepQA
# output: iqa/img_{compressor}_{field_name}.csv
cd iqa_metrics
python getIQA.py 
```

L2PIPS and DeepQA, two DNN-based IQA metrics, require external codebases as well as trained DNN models to compute. Those codebases are contained under **iqa_metrics/models**, which are fetched from

- L2PIPS: https://github.com/YuLvS/L2PIPS
- DeepQA: https://github.com/anse3832/DeepQA-modified

Calculated metrics values are output to `output_deepQA.csv` and `output_L2PIPS.csv`.

```bash
# for DeepQA metric
# output: output_deepQA.csv
cd iqa_metrics
python test_DeepQA.py 

# for L2PIPS metric
# output: test_L2PIPS
cd iqa_metrics
python test_L2PIPS.py 
```

**Note**: in order to run for L2PIPS, a previously trained DNN model needs to be downloaded and put into iqa_metrics/models/L2PIPS/trained_model. The trained model can be found [here](https://drive.google.com/drive/folders/1ClTLIOJrrXX5i_h-EZU9LkXEh5TDAAuc?usp=sharing).


# Contact
Jinzhen Wang, jinzhenw@lanl.gov
Pascal Grosset, pascalgrosset@lanl.gov

[![Build Status](https://travis-ci.org/lanl/VizAly-Vis_IO.svg?branch=master)](https://travis-ci.org/lanl/VizAly-Vis_IO)


# Copyright and License
This software is open source software available under the BSD-3 license.

Copyright (c) 2017, Triad National Security, LLC. All rights reserved.

This program was produced under U.S. Government contract 89233218CNA000001 for Los Alamos National Laboratory (LANL), which is operated by Triad National Security, LLC for the U.S. Department of Energy/National Nuclear Security Administration. The U.S. Government has rights to use, reproduce, and distribute this software. NEITHER THE GOVERNMENT NOR LOS ALAMOS NATIONAL SECURITY, LLC MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS SOFTWARE. If software is modified to produce derivative works, such modified software should be clearly marked, so as not to confuse it with the version available from LANL.

All rights in the program are reserved by Triad National Security, LLC, and the U.S. Department of Energy/National Nuclear Security Administration. The Government is granted for itself and others acting on its behalf a nonexclusive, paid-up, irrevocable worldwide license in this material to reproduce, prepare derivative works, distribute copies to the public, perform publicly and display publicly, and to permit others to do so.
