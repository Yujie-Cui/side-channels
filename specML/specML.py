# Reads in the HPC data collected with hpc-profiler for training

import glob
import Pandas as pd

# Read in HPC data from .csv files
for csv_file in glob.glob('data/*.csv'):
    # create a dataframe
    df = pd.read_csv(csv_file)

    # check if data is correct
    df.head()

    # get training data (inputs into the neural network)
