# Python GenericIO setup
The following python package are needed for the python.



```
module load anaconda/Anaconda3

if [ $? == 1 ]; then
    echo "Conda env not found, onetime setup.."
    conda create --yes --name cbench python=3.6

    # install Python packages
    conda install --yes numpy==1.15.4 matplotlib==3.0.2
    python -m pip install apsw==3.9.2.post1
    python -m pip install cv2
    python -m pip install scipy
    python -m pip install pandas
    python -m pip install dask
    python -m pip -install dask distributed
fi
```