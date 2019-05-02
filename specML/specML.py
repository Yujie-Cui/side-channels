# Reads in the HPC data collected with hpc-profiler for training

import glob
import pandas as pd
import numpy as np

# Inputs to the neural network
inputs = ['PAPI_TOT_INS', 'PAPI_L3_TCM', 'PAPI_L3_TCA']

# Read in HPC data from .csv files
for csv_file in glob.glob('data/*.csv'):
    # create a dataframe
    df = pd.read_csv(csv_file)

    # check if data is correct
    df.head()

    # get training data (inputs into the neural network)
    drop_data = []
    for column in list(df):
        if column not in inputs:
            drop_data.append(column)
    print('data to drop', drop_data)
    
    train_x = df.drop(drop_data)
    
    print('Training data')
    train_x.head()

    # create labels
    labels = None
    if 'spectre' in csv_file:
        labels = np.full((1, len(df)), 1)
    else:
        labels = np.full((1, len(df)), 0)
    print('Num rows', len(df))

    train_y = pd.DataFrame(labels, columns=['spectre'])
    print('Labels')
    train_y.head() 
